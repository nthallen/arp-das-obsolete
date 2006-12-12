/*
dc.c - data clients.
This module contains functions common to all dbr clients.
 $Log$
 Revision 1.2  1997/04/20 02:10:06  nort
 Nort's mods for non-blocking buffer clients

 Revision 1.1  1997/04/15 18:59:55  nort
 Initial revision

 * Revision 1.5  1994/03/04  21:08:06  eil
 * latest
 *
Written by DS
Modified 5/23/91 by NTA
Modified 9/26/91 by Eil, changed from ring to buffered ring. (dbr).
Modified extensivley and ported to QNX 4 by Eil 4/22/92.
Modified 4/97 by NTA for non-blocking buffered clients
*/

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <i86.h>
#include <signal.h>
#include <errno.h>
#include <signal.h>
#include <sys/kernel.h>
#include <sys/proxy.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/psinfo.h>
#include <sys/types.h>
#include <eillib.h>
#include <dbr.h>
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

/* Static variables:
	if reply_pid > 0, a reply is required
	got_tok is now a boolean indicating we're holding the token
	There are now 4 possible client_type/mod_type's
					DRC      DBC     DBCP     DBCC
	 Get Data via:  Recv    Send     Send     Recv
	 Forward Data:  Yes      No      Yes      No
	 Must Reply:    Yes      No      No       Yes
 */
static sigset_t sigs = 0L;
static pid_t reply_pid = 0;
static int got_tok = 0;
static int ds_tid = 0;
static unsigned int msg_size, dr_msg_size;
static unsigned char bow_out = 0;
static unsigned char oper_loop;
static dr_msg_type *dr_msg;
static token_type mf_row;
static pid_t my_pid;
static struct {
	msg_hdr_type msg_type;
	token_type n_rows;
} dr_tok = { DCTOKEN,0 };

/*
  Table of Contents:
  int DC_init(int client_type, int node);
  int DC_operate(void);
  int DC_bow_out(void);
  void DC_breakfunction(int);
  void DC_exitfunction(void);
  static int holding_token(void);
*/

/* the exit function during ring operation ensuring proper bow-out */
void DC_exitfunction(void) {
  pid_t pr;
  if (oper_loop) {
	breakfunction(0);
	if (!bow_out) {
	  if (dbr_info.mod_type == DRC) {
		holding_token();
		dr_tok.n_rows = 0;
		pr=qnx_proxy_attach(ds_tid,(char *)&dr_tok,sizeof(dr_tok),-1);
		while (Trigger(pr)==-1)
		  if (errno==EINTR) continue;
		  else return;
	  }
	  DC_bow_out();
	  DC_operate();
	}
  }
  msg(MSG,"task %d: DC operations completed",getpid());
}

/* initialises a client into the dbr.
Arguments: client_type - ring or buffered client, DRC, DBC or DBCX.
node - is node a buffered client can find the db, 0 means none specified.
Returns: Exits if fatal error occurs. zero on success.
*/
int DC_init(int client_type, nid_t node) {
  int pri;
  msg_hdr_type dr_init = DCINIT;
  struct _mxfer_entry mlist[2];

  oper_loop=1;
  bow_out = 0;
  breakfunction(0);
  my_pid=getpid();
  sigemptyset(&sigs);
  sigaddset(&sigs,SIGINT);
  sigaddset(&sigs,SIGTERM);
  alarmfunction(0);

  switch (client_type) {
	case DRC: case DBC: case DBCP: break;
	default: return 0;
  }
  
  oper_loop=0;

  /* ds_tid is source of data for a client, either dg or db. */
  if (client_type != DRC) {
	if ((ds_tid=qnx_name_locate(node, LOCAL_SYMNAME(DB_NAME), 0, 0)) == -1)
	  msg(MSG_EXIT_ABNORM,"Can't find symbolic name for %s on node %d",DB_NAME, node);
	pri = getprio(0);
	if (--pri>0) setprio(getpid(),pri);
  }  else  {
	if ((ds_tid=qnx_name_locate(0, GLOBAL_SYMNAME(DG_NAME), 0, 0)) == -1)
	  msg(MSG_EXIT_ABNORM,"Can't find symbolic name for %s",DG_NAME);
	qnx_pflags(~0,_PPF_SIGCATCH,0,0);
  }

  if (ds_tid==my_pid) msg(MSG_EXIT_ABNORM,"My data source can't be myself");

  /* get dbr_info */
  _setmx(&mlist[0], &dr_init, sizeof(msg_hdr_type));
  _setmx(&mlist[1], &dbr_info, sizeof(dbr_info));
	
  /* register data client exit function */
  if (atexit(DC_exitfunction))
	msg(MSG_EXIT_ABNORM,"Can't register DC exit function");

  while (!breaksignal)
	if ( !(oper_loop=!Sendmx(ds_tid, 1, 2, &mlist, &mlist)) )
	  if (errno != EINTR)
		msg(MSG_EXIT_ABNORM,"Error sending to my data source task %d",ds_tid);
	  else continue;
	else break;

  if (!breaksignal) {
	if (dr_init != DAS_OK) {
	  oper_loop = 0;
	  msg(MSG_EXIT_ABNORM,"bad response from data source task %d at registration",ds_tid);
	}
	/* make space for data transfers */
	dbr_info.mod_type = client_type;
	dr_msg_size = dbr_info.max_rows * dbr_info.tm.nbrow + sizeof(msg_hdr_type) + sizeof(dbr_data_type);
	if (dr_msg_size < sizeof(dr_msg_type)) dr_msg_size = sizeof(dr_msg_type);
	if ( !(dr_msg = malloc(dr_msg_size)))
	  msg(MSG_EXIT_ABNORM,"Can't allocate %d bytes of memory",dr_msg_size);
	if (!DC_data_rows) DC_data_rows = dbr_info.max_rows;
	else if (DC_data_rows>dbr_info.max_rows) {
	  msg(MSG_WARN,"ÿmin data msg size %d > %d allowable, defaulted",DC_data_rows,dbr_info.max_rows);
	  DC_data_rows=dbr_info.max_rows;
	}
  }
  else msg(MSG_EXIT_NORM,"caught signal %d: exiting",breaksignal);
  if ( client_type == DBCP ) {
	pid_t dbc_child = fork();
	switch (dbc_child) {
	  case 0:
		dbr_info.mod_type = DBCC;
		dbr_info.next_tid = 0;
		break;
	  default:
		dbr_info.next_tid = dbc_child;
		DC_operate(); /* Short circuit the rest of init */
		exit(0);
	  case -1:
		msg( MSG_EXIT_ABNORM, "Error forking" );
	}
  }
  return 0;
}

static void replyback(void) {
  if ( reply_pid <= 0 ) return;
  while (Reply(reply_pid, &my_pid, sizeof(pid_t))==-1 && errno==EINTR);
}

/* holding_token() handles all states while we have the DRing token.
   In most cases, the message or token should be immediately forwarded,
   but in rate-limiting cases, we sit here for awhile.
*/
static int holding_token(void) {
  pid_t rcv_buf, send_tid;

  if ( ! got_tok ) return 0;
  got_tok = 0;
  if ( ds_tid == 0 ||
	   dbr_info.mod_type == DBC || dbr_info.mod_type == DBCC )
	return 0;
  switch (dr_msg->msg_type) {
	case DCDATA: case TSTAMP: case DCDASCMD: break;
	default: return 0;          /* other message */
  }

  if (dbr_info.next_tid != ds_tid) {
	while (!breaksignal) {
	  send_tid = Send(dbr_info.next_tid,dr_msg,&rcv_buf,msg_size,
						 sizeof(pid_t));
	  if ( send_tid != -1 || errno != EINTR ) break;
	}
  } else {
	if (dbr_info.tm_started) {
	  /* data regulation */
	  while (!breaksignal && !DC_data_rows) {
		rcv_buf = Receive(0, dr_msg, dr_msg_size);
		if (rcv_buf == -1) {
		  if (errno != EINTR) break;
		  else continue;
		} else switch (dr_msg->msg_type) {
		  case DCDATA: case TSTAMP: case DCDASCMD: 
			msg(MSG_WARN,"Invalid Msg Received at Data Regulation Stage: type %d",dr_msg->msg_type);
			break;
		  default:
			/* Eventually this may reply with a message unknown */
			DC_other((unsigned char *)dr_msg, rcv_buf); 
			break;
		}
	  }
	  dr_tok.n_rows = (DC_data_rows==0) ? 1 : DC_data_rows;
	  while (!breaksignal) {
		send_tid = Send(dbr_info.next_tid,&dr_tok,&rcv_buf,sizeof(dr_tok),
						   sizeof(rcv_buf));
		if ( send_tid != -1 || errno != EINTR ) break;
	  }
	} else {
	  /* don't forward token unless TM started */
	  rcv_buf = ds_tid;
	}
  }

  if (breaksignal) return 0;

  /* pass on of token failed */
  if ( send_tid == -1 ) {
	if (errno==ESRCH) {           /* couldn't find neighbor */
	  /* If it was the DG, quit. Otherwise send to DG from here on */
	  if (dbr_info.next_tid == ds_tid) {
		oper_loop = 0;          /* no need to bow out */
		return -1;
	  } else if ( dbr_info.mod_type == DBCP ) {
		msg(MSG,"my child task %d terminated", dbr_info.next_tid );
		DC_bow_out();
	  } else {
		if (dbr_info.next_tid)
		  msg(MSG,"my ring neighbor task %d bowed out",dbr_info.next_tid);
		dbr_info.next_tid = ds_tid;
	  }
	} else got_tok = -1;
  } else if (rcv_buf != dbr_info.next_tid) {
	if ( dbr_info.mod_type == DBCP ) {
	  msg(MSG,"my child task %d terminated gracefully", dbr_info.next_tid );
	  DC_bow_out();
	} else {
	  dbr_info.next_tid = rcv_buf;
	  if (!rcv_buf) {
		dbr_info.next_tid=ds_tid;
		got_tok = 1;
		holding_token();
	  }
	  msg(MSG,"my new neighbor task is %d",dbr_info.next_tid);
	}
  }
  return 0;
}

/* client operation. returns 0 on success */
int DC_operate(void) {
  int rtrn_code = 0;            /* Code returned to calling function */
  int bower;
  pid_t send_tid;

  while (oper_loop && !breaksignal) {
	bower = 0;
	dr_msg->msg_type = DEATH;
	dr_msg->u.n_rows = DEATH;   
	/* send for data */
	switch ( dbr_info.mod_type ) {
	  case DBC:
	  case DBCP:
		if (!DC_data_rows && !bow_out) {
		  msg(MSG_WARN,"null request, bowing out");
		  DC_bow_out();
		}
		if (bow_out) {
		  DC_data_rows=0;         /* send null token */
		  bower = 1;
		}
		dr_tok.n_rows=DC_data_rows;
		send_tid =
		  Send( ds_tid, &dr_tok, dr_msg, sizeof(dr_tok), dr_msg_size );
		if ( send_tid == -1 ) {
		  if (errno != EINTR) goto breakout;
		  else continue;
		}
		got_tok = 1;
		break;
	  case DRC:
	  case DBCC:
		/* receive for data */
		reply_pid = Receive(0, dr_msg, dr_msg_size);
		if (reply_pid == -1) {
		  if (errno != EINTR) goto breakout;
		  else continue;
		}
		got_tok = 1;
		/* Handle bow_out condition for ring clients */
		if (bow_out && !mf_row) {
		  switch (dr_msg->msg_type) {
			case DCDATA: case DCDASCMD: case TSTAMP:
			  bower = 1;
			  if (dbr_info.next_tid == ds_tid || dbr_info.next_tid == 0) {
				my_pid=0;
				replyback();
			  } else {
				while ( Relay( reply_pid, dbr_info.next_tid) == -1 &&
						errno==EINTR);
			  }
			default: break;
		  }
		}
		break;
	  default:
		msg(MSG_EXIT_ABNORM, "dbr_info.mod_type corrupted" );
	}

	if (!bower) {
	  /* switch on data header */
	  switch (dr_msg->msg_type) {
		case DEATH:
		  break;
		case DCDATA:
		  replyback();
		  msg_size = dr_msg->u.drd.n_rows * tmi(nbrow) + sizeof(msg_hdr_type)
					  + sizeof(token_type);
		  if (dr_msg->u.drd.n_rows) {
			sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
			DC_data(&dr_msg->u.drd);
			sigprocmask(SIG_UNBLOCK,&sigs,0);
			mf_row = (mf_row + dr_msg->u.drd.n_rows) % dbr_info.nrowminf;
		  }
		  rtrn_code = holding_token();
		  break;
		case TSTAMP:
		  assert(!mf_row);
		  replyback();
		  dbr_info.t_stmp = dr_msg->u.tst;
		  msg_size = sizeof(tstamp_type) + sizeof(msg_hdr_type);
		  sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
		  DC_tstamp(&dr_msg->u.tst);
		  sigprocmask(SIG_UNBLOCK,&sigs,0);
		  rtrn_code = holding_token();
		  break;
		case DCDASCMD:
		  assert(!mf_row);
		  replyback();
		  msg_size = sizeof(dascmd_type) + sizeof(msg_hdr_type);
		  switch (dr_msg->u.dasc.type) {
			case DCT_TM:
			  switch (dr_msg->u.dasc.val) {
				case DCV_TM_START:
				  dbr_info.tm_started = 1; break;
				case DCV_TM_END:
				  dbr_info.tm_started = 0; break;
				default: break;
			  }
			  break;
			case DCT_QUIT:
			  switch (dr_msg->u.dasc.val) {
				case DCV_QUIT:
				  switch (dbr_info.mod_type) {
					/* ring clients don't bow out after QUIT */
					case DRC: oper_loop = 0; break;
					case DBC:
					case DBCP:
					case DBCC: DC_bow_out(); break;
					default: msg(MSG_EXIT_ABNORM,"yikes who are you?");
				  }
				  break;
				default: break;
			  }
			default: break;
		  }
		  sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
		  DC_DASCmd(dr_msg->u.dasc.type, dr_msg->u.dasc.val);
		  sigprocmask(SIG_UNBLOCK,&sigs,0);
		  rtrn_code = holding_token();
		  break;
		default: 
		  /* DC_other responsible for reply */
		  if ( dbr_info.mod_type == DBC || dbr_info.mod_type == DBCP )
			msg(MSG_EXIT_ABNORM,
			  "Unexpected message type from bfr" );
		  sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
		  DC_other((unsigned char *)dr_msg, reply_pid);
		  break; 
	  }                         /* switch */
	} else {
	  oper_loop=0;
	  ds_tid=0;
	}
	sigprocmask(SIG_UNBLOCK,&sigs,0);
  }                             /* while */
  breakout:

  if (breaksignal) msg(MSG,"caught signal %d: disengaging",breaksignal);
  else if (got_tok==-1) {
	oper_loop = 0;
	rtrn_code = -1;
	msg(MSG_WARN,"error getting data");
  }
  return rtrn_code;
}

/* disengage gracefully from dbr */
int DC_bow_out(void) {
  if (ds_tid==0) oper_loop = 0;
  if (bow_out) return 0;
  msg(MSG,"task %d: bowing out",getpid());
  bow_out=1;
  return 0;
}
