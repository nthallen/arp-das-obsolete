/*
	dc.c - data clients.
	This module contains functions common to all dbr clients.
	Written by DS
	Modified 5/23/91 by NTA
	Modified 9/26/91 by Eil, changed from ring to buffered ring. (dbr).
	Modified extensivley and ported to QNX 4 by Eil 4/22/92.
*/

/*
	Notes:
	Every client receives data, timestamps and dascmds.
	Every client receives msgs with headers DCDATA, TSTAMP, DCDASCMD.
	Every client is either a ring client or a buffered client.
	Every client either waits for data (receive) or requests data (send).
	The main loop of DC_operate is protected from BREAKS.
*/

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <das_utils.h>
#include <dbr_utils.h>
#include <globmsg.h>

/* Structures for this particular module */
typedef struct {
  msg_hdr_type msg_type;
  union {
    dbr_data_type drd;
    tstamp_type tst;
    dascmd_type dasc;
    token_type n_rows;
  } u;
} dr_msg_type;

/* Globals */
token_type DC_data_rows;

/* Static variables */
static int got_tok = 0;
static int ds_tid = 0;
static unsigned int msg_size, dr_msg_size;
static unsigned char bow_out = 0;
static unsigned char oper_loop = 1;    /* termination condition */
static unsigned char got_quit = 0;    /* QUIT received ? */
static dr_msg_type *dr_msg, *rq_msg;
static token_type mf_row;
static pid_t my_pid;
static struct {
    msg_hdr_type msg_type;
    token_type n_rows;
} dr_tok = { DCTOKEN,0 };

/*
  Table of Contents:
  int DC_init(int client_type, int node, int bclient_type);
  int DC_operate(void);
  int DC_bow_out(void);
  void DC_exitfunction(void);
  static int holding_token(void);
*/


/* the exit function during ring operation ensuring proper bow-out */
void DC_exitfunction(void) {
/* star clients bow out after QUIT, ring clients dont */
if (dbr_info.mod_type == DRC && got_quit) return;
    if (!bow_out) {
	if (DC_bow_out() !=0)
	    msg(MSG_WARN,"error bowing out");
	if (dbr_info.mod_type == DRC && got_tok) holding_token();
	DC_operate();
    }
}

/* initialises a client into the dbr.
Arguments:
	client_type - ring or buffered client.
	node - is the node a buffered client can find the db, 0 means none specified.
Returns:
	Exits if fatal error occurs.
	zero on success.
*/
int DC_init(int client_type, nid_t node) {
  int i, pri;
  nid_t n;
  char nm[16];
  char name[FILENAME_MAX+1];
  msg_hdr_type dr_init = DCINIT;
  struct _mxfer_entry mlist[3];

  assert(client_type == DRC || client_type == DBC);

  /* error handling intialisation if the client code didnt */
  if (!msg_initialised())
	msg_init("client",0,1,-1,0,1,1);

  /* initialize with the dbr */
  if (client_type == DBC) {
	strncpy(nm,DB_NAME,8);
	n = node;
	pri = getprio(0);
	if (--pri>0) setprio(getpid(),pri);
  }  else  {
	strncpy(nm,DG_NAME,8);
	n = getnid();
  }
  /*
	ds_tid is source of data for a client, either a dg or a db.
	for a ring client : ds_tid is the data generator on same node.
	for a buffered client : ds_tid is a data buffer anywhere.
  */
  for (i=0;i<3;i++)
    if ((ds_tid=qnx_name_locate(n, LOCAL_SYMNAME(nm,name), 0, 0)) == -1) {
	if (!i) msg(MSG,"Im trying to find %s on node %d", name, n);
	sleep(1);
    }
    else break;
  if (i>0 && i<3) {
    errno=0;
    msg(MSG,"Found %s on node %d, continuing...",name,node);
  }
  else if (i>=3) msg(MSG_EXIT_ABNORM,"Couldn't find %s on node %d",name,n);

  if (ds_tid==getpid()) msg(MSG_EXIT_ABNORM,"My data source can't be myself");

  /* get dbr_info */
  _setmx(&mlist[0], &dr_init, sizeof(msg_hdr_type));
  _setmx(&mlist[1], &mf_row, sizeof(token_type));
  _setmx(&mlist[2], &dbr_info, sizeof(dbr_info));
  if ( (i = Sendmx(ds_tid, 1, 3, &mlist, &mlist)) == -1 )
    msg(MSG_EXIT_ABNORM,"Error sending to my data source %s",nm);

  if (dr_init != DAS_OK)
    msg(MSG_EXIT_ABNORM,"reply from DG task %d was (%d!=DAS_OK)",ds_tid,dr_init);

  /* register data client exit function */
  if (atexit(DC_exitfunction))
    msg(MSG_EXIT_ABNORM,"Can't register DC exit function");

  /* make space for data transfers */
  dr_msg_size = dbr_info.max_rows * dbr_info.tm.nbrow + sizeof(msg_hdr_type) + sizeof(dbr_data_type);
  if (dr_msg_size < sizeof(dr_msg_type))
    dr_msg_size = sizeof(dr_msg_type);
  dr_msg = malloc(dr_msg_size);
  if (!DC_data_rows) DC_data_rows = dbr_info.max_rows;
  else if (DC_data_rows>dbr_info.max_rows) {
    msg(MSG_WARN,"ÿmin data msg size %d > %d allowable, defaulted",DC_data_rows,dbr_info.max_rows);
    DC_data_rows=dbr_info.max_rows;
  }

	    if (dr_msg_size < sizeof(dr_msg_type)) dr_msg_size = sizeof(dr_msg_type);
  if (client_type == DBC)
    if ( !(rq_msg=malloc(sizeof(dr_msg_type))))
	msg(MSG_EXIT_ABNORM,"Can't allocate memory for request msgs");
    else rq_msg->msg_type=DCTOKEN;

  my_pid=getpid();
  BREAK_SETUP;
  return 0;
}

/* holding_token() handles all states while we have the DRing token.
   In most cases, the message or token should be immediately forwarded,
   but in rate-limiting cases, we sit here for awhile.

   Refers to the static variables:
     dr_msg   (when forwarding)
     msg_size (to know how much to forward)
     dr_tok   (when sending token)
     oper_loop (to terminate on error conditions)
*/
static int holding_token(void) {
  int back;            /* returns from Sends and Receives */
  pid_t rcv_buf;      /* Buffer to place receive characters */

  got_tok = 0;
  for (;;) {
    if (dbr_info.next_tid != ds_tid)
      back = Send(dbr_info.next_tid, dr_msg, &rcv_buf,
                 msg_size, sizeof(pid_t));
    else {
      if (dbr_info.tm_started) while (DC_data_rows==0) {
	back = Receive(0, dr_msg, dr_msg_size);
        switch (dr_msg->msg_type) {
          case DCDATA:
          case TSTAMP:
          case DCDASCMD:
            oper_loop = 0;
            return -1;
          default:
            /* Eventually this may reply with a message unknown */
            DC_other((unsigned char *)dr_msg, rcv_buf); 
            break;
        }
      }
      dr_tok.n_rows = DC_data_rows;
      back = Send(dbr_info.next_tid, &dr_tok, &rcv_buf,
                 sizeof(dr_tok), sizeof(rcv_buf));
    }

    /* If send failed, assume next tid is down */
    if (back == -1) {
	errno = 0;
	/* If it was the DG, quit. Otherwise send to DG from here on */
	if (dbr_info.next_tid == ds_tid) {
	    oper_loop = 0;
	    return -1;
	    break;
	} else {
	    if (dbr_info.next_tid)
		msg(MSG,"my neighbor task %d bowed out from node %d ring",dbr_info.next_tid, getnid());
	    dbr_info.next_tid = ds_tid;
	}
    } else {
	if (rcv_buf != dbr_info.next_tid) dbr_info.next_tid = rcv_buf;
	break;
    }
  }  /* for */

  return 0;
}

int DC_operate(void) {
int who;
int rtrn_code = 0;  /* Code returned to calling function */

   for (oper_loop = 1; oper_loop==1; ) {

    BREAK_PROTECT;
    /* get the data */
    if (dbr_info.mod_type == DBC) {  /* send for data */
	if (!DC_data_rows && !bow_out) {
	    msg(MSG_WARN,"null request, bowing out");
	    DC_bow_out();
	}
	if (bow_out) {  /* send a null request to DB */
	    DC_data_rows=0;
	    oper_loop=0;
	}
	/* make request */
	rq_msg->u.n_rows=DC_data_rows;
	if ( (Send(ds_tid, rq_msg, dr_msg, sizeof(msg_hdr_type)+sizeof(token_type), dr_msg_size))==-1)
	    /* most likely, bfr crashed */
	    msg(MSG_EXIT_ABNORM,"cannot request from task %d",ds_tid);
    } else {  /* receive for data */
	while ( (who = Receive(0, dr_msg, dr_msg_size))==-1)
	    msg(MSG_WARN,"error on receive");
	/* Handle bow_out condition for ring clients */
	if (bow_out) {
	    oper_loop = 0;
	    if (dbr_info.next_tid == ds_tid) {
		my_pid=0;
		Reply(who, &my_pid, sizeof(pid_t));
	    } else Relay(who, dbr_info.next_tid);
	}
    }

    if (!bow_out)  {
	/* switch on data header */    	
        switch (dr_msg->msg_type) {
	    case DCDATA:
		got_tok = 1;
		if (dbr_info.mod_type==DRC) Reply(who, &my_pid, sizeof(pid_t));
		msg_size = dr_msg->u.drd.n_rows * tmi(nbrow) + sizeof(msg_hdr_type) + sizeof(token_type);
		if (!mf_row) DC_data(&dr_msg->u.drd);
		else {
		    assert(mf_row < dbr_info.nrowminf);
		    mf_row = (mf_row + dr_msg->u.drd.n_rows) % dbr_info.nrowminf;
		}
		if (dbr_info.mod_type==DRC) rtrn_code = holding_token();
	    break;
	    case TSTAMP:
		got_tok = 1;
		assert(!mf_row);
		if (dbr_info.mod_type==DRC) Reply(who, &my_pid, sizeof(pid_t));
		dbr_info.t_stmp = dr_msg->u.tst;
		msg_size = sizeof(tstamp_type) + sizeof(msg_hdr_type);
		DC_tstamp(&dr_msg->u.tst);
		if (dbr_info.mod_type==DRC) rtrn_code = holding_token();
	    break;
	    case DCDASCMD:
		got_tok = 1;
		assert(!mf_row);
		if (dbr_info.mod_type==DRC) Reply(who, &my_pid, sizeof(pid_t));
		if ((dr_msg->u.dasc.type == DCT_QUIT) && (dr_msg->u.dasc.val == DCV_QUIT)) {
		    oper_loop = 0;
		    got_quit = 1;
		}
		if (dr_msg->u.dasc.type == DCT_TM)
		    if (dr_msg->u.dasc.val == DCV_TM_START)
			dbr_info.tm_started = 1;
		    else if (dr_msg->u.dasc.val == DCV_TM_END)
			dbr_info.tm_started = 0;
 		msg_size = sizeof(dascmd_type) + sizeof(msg_hdr_type);
		DC_DASCmd(dr_msg->u.dasc.type, dr_msg->u.dasc.val);
		if (dbr_info.mod_type==DRC) rtrn_code = holding_token();
          break;
        default: 
	    /* DC_other responsible for reply */
	    DC_other((unsigned char *)dr_msg, who); break; 
      } /* switch */
    } /* if */
    BREAK_ALLOW;
  } /* for */

  BREAK_ALLOW;
  return rtrn_code;
}

/* disengage gracefully from dbr */
int DC_bow_out(void) {
  msg(MSG,"task %d: bowing out",getpid());
  bow_out=1;
  return 0;
}


