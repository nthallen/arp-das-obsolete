/*
 dg.c defines the routines for the distributor portion of the
 data generator. These are the routines common to all DG's.
 Written by NTA April 24, 1991
 Modified Aug 20, 1991 by Eil, to attach data_gen name and register with cmdctrl.
 Modified Aug 27, 1991 by Eil, so some task gets dg as next_tid.
 Modified Sep 26, 1991 by Eil, changing from ring to buffered ring.
 Modified and ported to QNX 4 4/23/92 by Eil.
 $Log$
 * Revision 1.5  1992/06/09  20:36:12  eil
 * dbr_info.seq_num and dr_forward
 *
 * Revision 1.4  1992/06/09  14:34:32  eil
 * during star development
 *
 * Revision 1.3  1992/05/22  20:26:35  eil
 * eil, sends and receives on ring
 *
 * Revision 1.2  1992/05/20  17:24:53  nort
 * Modified DG_init() not to read .dac file
 * (extracted .dac input to dgdacin.c DG_dac_in())
 * Modified DG_init_options() to use global opt_string
 *  and fixed bugs surrounding getopt().
 *
 * Revision 1.1  1992/05/19  14:09:02  nort
 * Initial revision
 *
*/

static char rcsid[] = "$Id$";

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/sendmx.h>
#include <globmsg.h>
#include <das_utils.h>
#include <dbr_utils.h>

/* Preprocessor definitions: */
#define ANY_TASK 0
#define DASCQSIZE 5

/* Structures for this particular module */
typedef struct {
  msg_hdr_type msg_type;
  union {
    token_type n_rows; /* for token */
    dascmd_type dasc;
  } u;
  char fill[MAX_MSG_SIZE];
} dg_msg_type;

/* Globals */
int dg_id;
static int clients_inited = 0;
static int start_at_clients = 0;
static dg_msg_type dg_msg;
static unsigned int holding_token = 1;
static token_type DG_rows_requested = 0;
static token_type adjust_rows = 0;
static unsigned int nrowminf;
static unsigned int oper_loop;
static unsigned int minf_row;
static pid_t my_pid;
static struct {
  struct {
    unsigned char type;
    unsigned char val;
  } q[DASCQSIZE];
  unsigned int next;
  unsigned int ncmds;
} dasq;


static int init_client(pid_t who) {
  struct _mxfer_entry mlist[2];

  _setmx(&mlist[0],&minf_row, sizeof(token_type));
  if (!dbr_info.next_tid) {
    dbr_info.next_tid = getpid();
    holding_token=1;
  }
  _setmx(&mlist[1],&dbr_info,sizeof(dbr_info));
  if (!(Replymx(who, 2, mlist))) {
    dbr_info.next_tid = who;
    adjust_rows = dbr_info.nrowminf-minf_row-1;
    if (++clients_inited == start_at_clients)
	dbr_info.tm_started = 1;
  }
  return 0;
}

/* returns TRUE if the queue is full */
static int q_DAScmd(unsigned char type, unsigned char val) {
  unsigned int new;

  if (dasq.ncmds >= DASCQSIZE) return 1;
  new = (dasq.next + dasq.ncmds) % DASCQSIZE;
  dasq.q[new].type = type;
  dasq.q[new].val = val;
  dasq.ncmds++;
  return 0;
}

/* returns TRUE if queue is non-empty and on a mf boundary, FALSE if empty */
static int dq_DAScmd(dascmd_type *dasc) {
  if (dasq.ncmds == 0) return 0;
  if (minf_row) {
    adjust_rows=dbr_info.nrowminf-minf_row-1;
    return(0);
  }
  dasc->type = dasq.q[dasq.next].type;
  dasc->val = dasq.q[dasq.next].val;
  dasq.next++;
  if (dasq.next == DASCQSIZE) dasq.next = 0;
  dasq.ncmds--;
  return(1);
}

/* dr_forward() forwards to the next tid. */
static void dr_forward(msg_hdr_type msg_type, token_type n_rows,
		       void *other, token_type n_rows1, void *other1) {
  static pid_t rval;
  static struct _mxfer_entry slist[4];
  static struct _mxfer_entry rlist;
  token_type tmp;
  int scount;

  if (dbr_info.next_tid == 0) return;
  _setmx(&rlist,&rval,sizeof(pid_t));
  _setmx(&slist[0],&msg_type,sizeof(msg_hdr_type));
  scount=2;
  switch(msg_type) {
    case DCDATA:
	assert(n_rows);
	tmp=n_rows;
	if (other1 && n_rows1) tmp+=n_rows1;
	_setmx(&slist[1],&tmp,sizeof(token_type));
	_setmx(&slist[2],other,n_rows*tmi(nbrow));
	scount=3;
	if (other1 && n_rows1) {
	_setmx(&slist[3],other1,n_rows1*tmi(nbrow));
	    scount=4;
	}
	break;
    case TSTAMP:
	_setmx(&slist[1],other,sizeof(tstamp_type));	
	break;
    case DCDASCMD:
	_setmx(&slist[1],other,sizeof(dascmd_type));	
	break;
    default: msg(MSG_EXIT_ABNORM,"in dr_forward, bad type %d",msg_type);
  }

  while (Sendmx(dbr_info.next_tid, scount, 1, slist, &rlist)==-1);
  if (rval != dbr_info.next_tid)
    dbr_info.next_tid = rval;
  holding_token = 0;
}


/* dist_DAScmd() determines whether the DAScmd is a dbr DAScmd or not.
   If it is, it queues it for execution when appropriate. Otherwise
   it call DG_other() immediately.
*/
static void dist_DAScmd(dg_msg_type *msg, pid_t who) {
  unsigned char rep_msg = DAS_OK, is_dr = 0;

  switch (msg->u.dasc.type) {
    case DCT_QUIT:
      if (msg->u.dasc.val == DCV_QUIT) is_dr = 1;
      break;
    case DCT_TM:
      switch (msg->u.dasc.val) {
        case DCV_TM_START:
        case DCV_TM_END:
        case DCV_TM_CLEAR:
        case DCV_TM_SUSLOG:
        case DCV_TM_RESLOG:
          is_dr = 1;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  if (is_dr) {
    if (q_DAScmd(msg->u.dasc.type, msg->u.dasc.val)) rep_msg = DAS_BUSY;
    Reply(who, &rep_msg, 1);
  } else DG_other((unsigned char *)msg, who);
}

/* dist_DCexec() executes dbr DAScmds and forwards them on the ring.
   Returns TRUE if the command is QUIT.
*/
static int dist_DCexec(dascmd_type *dasc) {
  if (dasc->type == DCT_TM) {
    if (dasc->val == DCV_TM_START) {
      dbr_info.tm_started = 1;
      minf_row = 0;
    } else if (dasc->val == DCV_TM_END)
      dbr_info.tm_started = 0;
  }
  DG_DASCmd(dasc->type, dasc->val);
  dr_forward(DCDASCMD, 0, dasc, 0,0);
  return (dasc->type == DCT_QUIT && dasc->val == DCV_QUIT);
}

void DG_exitfunction(void) {
    if (oper_loop) {
	holding_token=1;
	DG_s_dascmd(DCT_QUIT,DCV_QUIT);
    }
    return;
}

int DG_init_options(int argcc, char **argvv) {
extern char *optarg;
extern int optind, opterr, optopt;
char filename[FILENAME_MAX] = {'\0'};
int c,s;

    /* error handling intialisation if the client code didnt */
    if (!msg_initialised())
	msg_init(DG_NAME,0,1,0,0,1,1);

    s = 0;
    opterr = 0;
    optind = 0;

    do {
	  c=getopt(argcc,argvv,opt_string);
	  switch (c) {
		case 'n': s = atoi(optarg); break;
		case '?':
		  msg(MSG_EXIT_ABNORM, "Invalid option -%c", optopt);
		default : break;
	  }
	} while (c != -1);
    optind = 0;
    opterr = 1;
    return(DG_init(s));
}


/* DG_init() performs initializations:
    Performs sanity checks on dbr_info.
    Initializes remainder of the dbr_info structure.
    Exits on an error we can't continue with.
    Returns non-zero if there is a problem, but we can continue.
*/
int DG_init(int s) {
  unsigned char rv = 0;
  int t;
  unsigned char replycode;
  char name[FILENAME_MAX+1];

  /* error handling intialisation if the client code didnt */
  if (!msg_initialised())
    msg_init(DG_NAME,0,1,0,0,1,1);

  /* attach name */
  if ((dg_id=qnx_name_attach(getnid(),LOCAL_SYMNAME(DG_NAME,name)))==-1)
    msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);  

  /* register data client exit function */
  if (atexit(DG_exitfunction))
    msg(MSG_EXIT_ABNORM,"Can't register DG exit function");

  /* initialize the remainder of dbr_info. tm info should already
     have been determined, either at compile time or by calling
	 DG_dac_in().
  */
  dbr_info.tm_started = 0;
  dbr_info.mod_type = DG;
  dbr_info.next_tid = 0;
  dbr_info.max_rows = (unsigned int)((MAX_BUF_SIZE-sizeof(msg_hdr_type)-sizeof(token_type))/tmi(nbrow));
  dbr_info.nrowminf = tmi(nbminf) / tmi(nbrow);
  /* tstamp remains undefined until TM starts. */
  start_at_clients = s;
  clients_inited=0;
  if (!dbr_info.max_rows) dbr_info.max_rows = 1;
  my_pid=getpid();
  /* initialize DAScmd queue: */
  dasq.next = dasq.ncmds = 0;
  return(rv);
}

int DG_operate(void) {
  int who;
  dascmd_type dasc;

  for (oper_loop = 1; oper_loop != 0; ) {
    do who = Receive(ANY_TASK, &dg_msg, sizeof(dg_msg)); while (who < 0);
      switch (dg_msg.msg_type) {
	case DCINIT: init_client(who);  break;
        case DASCMD:
          dist_DAScmd(&dg_msg, who);
          break;
        case DCTOKEN:
          assert(holding_token == 0);
          DG_rows_requested = dg_msg.u.n_rows;
          holding_token = 1;
          Reply(who, &my_pid, sizeof(pid_t));
          break;
        case DCDATA:
        case DCDASCMD:
        case TSTAMP:
          msg(MSG_WARN,"Invalid message received by DG of type %d",dg_msg.msg_type);
	  break;
        default:
          DG_other((unsigned char *)&dg_msg, who);
          break;
      }
    if (holding_token) {
	if (dq_DAScmd(&dasc)) {
          if (dist_DCexec(&dasc))
            oper_loop = 0;
        } else if (dbr_info.tm_started) {
	  if (adjust_rows) {
	    assert(adjust_rows<dbr_info.nrowminf);
	    DG_rows_requested -= DG_rows_requested%dbr_info.nrowminf - dbr_info.nrowminf + adjust_rows;
	    adjust_rows=0;
	  }
          if (DG_rows_requested == 0 || DG_rows_requested > dbr_info.max_rows)
            DG_rows_requested = dbr_info.max_rows;
          DG_get_data(DG_rows_requested);
	}
    }
  }
  return 0;
}


/* Called from DG_get_data(). */
void DG_s_data(token_type n_rows, unsigned char *data, token_type n_rows1, unsigned char *data1) {

  assert(holding_token);
  assert(n_rows != 0);
  assert(n_rows <= DG_rows_requested);
  dr_forward(DCDATA, n_rows, data, n_rows1, data1);
  if (dbr_info.nrowminf > 1)
    minf_row = (minf_row + n_rows) % dbr_info.nrowminf;
}


void DG_s_tstamp(tstamp_type *tstamp) {
  assert(holding_token);
  assert(!minf_row);
  dbr_info.t_stmp = *tstamp;
  dr_forward(TSTAMP, 0, tstamp, 0,0);
}


void DG_s_dascmd(unsigned char type, unsigned char val) {
  dascmd_type dasc;

  assert(holding_token);
  dasc.type = type;
  dasc.val = val;
  if (dist_DCexec(&dasc))
    oper_loop = 0;
}


