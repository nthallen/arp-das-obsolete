/*
 dg.c defines the routines for the distributor portion of the
 data generator. These are the routines common to all DG's.
 $Log$
 * Revision 1.15  1994/03/04  21:08:25  eil
 * latest
 *
 *
 * Modified and ported to QNX 4 4/23/92 by Eil.
 * Modified Sep 26, 1991 by Eil, changing from ring to buffered ring.
 * Modified Aug 27, 1991 by Eil, so some task gets dg as next_tid.
 * Modified Aug 20, 1991 by Eil, to attach data_gen name and register with cmdctrl.
 * Written by NTA April 24, 1991
 */

static char rcsid[] = "$Id$";

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <i86.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/psinfo.h>
#include <globmsg.h>
#include <eillib.h>
#include <dbr.h>

/* Preprocessor definitions: */
#define DASCQSIZE 5

/* Structures for this particular module */
typedef struct {
  msg_hdr_type msg_type;
  union {
    token_type n_rows; /* for token */
    dascmd_type dasc;
    char fill[MAX_MSG_SIZE]; /* for now */
  } u;
} dg_msg_type;

/* Statics */
static sigset_t sigs = 0L;
static pid_t initialise_client = 0;
static int clients_inited = 0;
static int start_at_clients = 0;
static int dg_delay = 0;
static dg_msg_type dg_msg;
static unsigned int holding_token = 0;
static token_type DG_rows_requested = 1;
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


void DG_exitfunction(void) {
  pid_t pr;
  if (dbr_info.next_tid)	/* there are clients */
    if (oper_loop) {
      /* trigger myself a quit */
      dg_msg.msg_type = DASCMD;
      dg_msg.u.dasc.type = DCT_QUIT;
      dg_msg.u.dasc.val = DCV_QUIT;
      pr=qnx_proxy_attach(0,(char *)&dg_msg,sizeof(dascmd_type)+sizeof(msg_hdr_type),-1);
      while (Trigger(pr)==-1 && errno==EINTR);
      breakfunction(0);
      DG_operate();
    }
  msg(MSG,"task %d: DG operations completed",getpid());    
}

static int init_client(pid_t who) {
  struct _mxfer_entry mlist[2];
  msg_hdr_type rv = DAS_OK;

  _setmx(&mlist[0],&rv, sizeof(msg_hdr_type));
  if (!dbr_info.next_tid) {
    if (dbr_info.tm_started) holding_token = 1;
    dbr_info.next_tid = getpid();
  }

  _setmx(&mlist[1],&dbr_info,sizeof(dbr_info));
  while (Replymx(who, 2, mlist)==-1)
    if (errno==EINTR) continue;
    else { msg(MSG_WARN,"error replying to task %d",who); return -1; }
  	
  dbr_info.next_tid = who;
  if (minf_row) adjust_rows = dbr_info.nrowminf-minf_row;
  if (++clients_inited == start_at_clients && !dbr_info.tm_started) {
    holding_token = 1;
    DG_s_dascmd(DCT_TM,DCV_TM_START);
  }
  msg(MSG,"task %d is my ring (node %d) client",who, getnid());
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

/* returns TRUE if queue is non-empty and on a mf boundary, FALSE otherwise */
static int dq_DAScmd(dascmd_type *dasc) {
  if (dasq.ncmds == 0) return 0;
  if (minf_row) {
    adjust_rows=dbr_info.nrowminf-minf_row;
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

  if (dbr_info.next_tid == 0) {
    holding_token=0;
    return;
  }
  _setmx(&rlist,&rval,sizeof(pid_t));
  _setmx(&slist[0],&msg_type,sizeof(msg_hdr_type));
  scount=2;
  switch(msg_type) {
  case DCDATA:
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
    /* the following would be a programmatic error */
  default: msg(MSG_EXIT_ABNORM,"can't forward msgs with type %d onto ring",msg_type);
    return;
  }

  sigprocmask(SIG_UNBLOCK,&sigs,0);
  while (!breaksignal)
    if ( (holding_token=Sendmx(dbr_info.next_tid, scount, 1, slist, &rlist)))
      if (errno!=EINTR) {
	oper_loop=0;
	msg(MSG_WARN,"error forwarding data to task %d",dbr_info.next_tid);
	dbr_info.next_tid = 0;
	break;
      } else continue;
    else break;
  sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
  if (holding_token) return;

  if (rval != dbr_info.next_tid) {
    msg(MSG,"my ring neighbor task %d bowed out",dbr_info.next_tid);
    dbr_info.next_tid = rval;
  }
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
      if (dbr_info.tm_started) is_dr = 2;
      else is_dr = 1;
      break;
    case DCV_TM_END:
    case DCV_TM_CLEAR:
    case DCV_TM_SUSLOG:
    case DCV_TM_RESLOG: is_dr = 1; break;
    default: break;
    }
    break;
  default: break;
  }
  switch (is_dr) {
  case 1:
    if (!dbr_info.tm_started) holding_token=1;
    if (q_DAScmd(msg->u.dasc.type, msg->u.dasc.val)) rep_msg = DAS_BUSY;
  case 2: while (Reply(who, &rep_msg, 1)==-1 && errno==EINTR); break;
  case 0: DG_other((unsigned char *)msg, who); break;
  }
}

/* dist_DCexec() executes dbr DAScmds and forwards them on the ring.
   Returns TRUE if the command is QUIT.
*/
static int dist_DCexec(dascmd_type *dasc) {
  if (dasc->type == DCT_TM) {
    if (dasc->val == DCV_TM_START) {
      dbr_info.tm_started = 1;
      minf_row = 0;
      holding_token = 1;
    } else if (dasc->val == DCV_TM_END)
      dbr_info.tm_started = 0;
  }
  sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
  DG_DASCmd(dasc->type, dasc->val);
  sigprocmask(SIG_UNBLOCK,&sigs,0);
  dr_forward(DCDASCMD, 0, dasc, 0,0);
  if (dasc->type == DCT_QUIT && dasc->val == DCV_QUIT) {
    dbr_info.next_tid=0;
    return 1;
  }
  return 0;
}

/* DG_init() performs initializations:
    Performs sanity checks on dbr_info.
    Initializes remainder of the dbr_info structure.
    Exits on an error we can't continue with.
    Returns zero on success.
*/
int DG_init(int s, int del) {
  int t;
  unsigned char replycode;

  assert(TIMEOUT > 0);

  /* register data generator exit function */
  if (atexit(DG_exitfunction))
    msg(MSG_EXIT_ABNORM,"Can't register DG exit function");

  /* attach name */
  if (qnx_name_attach(getnid(),GLOBAL_SYMNAME(DG_NAME))==-1)
    msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",DG_NAME);

  sigemptyset(&sigs);
  sigaddset(&sigs,SIGINT);
  sigaddset(&sigs,SIGTERM);

  /* initialize the remainder of dbr_info. tm info should already
     have been determined, either at compile time or by calling DG_dac_in().
     */
  dbr_info.tm_started = 0;
  dbr_info.mod_type = DG;
  dbr_info.next_tid = 0;
  dbr_info.max_rows = (unsigned int)((MAX_BUF_SIZE-sizeof(msg_hdr_type)-sizeof(token_type))/tmi(nbrow));
  dbr_info.nrowminf = tmi(nbminf) / tmi(nbrow);
  /* tstamp remains undefined until TM starts. */
  start_at_clients = s;
  dg_delay = (del < 1) ? 0 : del;
  clients_inited=0;
  if (!dbr_info.max_rows) dbr_info.max_rows = 1;
  DG_rows_requested = dbr_info.max_rows;

  /* misc initialisations */
  my_pid=getpid();
  qnx_pflags(~0,_PPF_SIGCATCH,0,0);
  /* initialize DAScmd queue: */
  dasq.next = dasq.ncmds = 0;
  oper_loop = 1;
  alarmfunction(0);
  return 0;
}

int DG_operate(void) {
  int who;
  dascmd_type dasc;
  int i,j;
	
  breakfunction(0);
	
  while (oper_loop && !breaksignal) {
    dg_msg.msg_type = DEATH;
    if ((who=Receive(0,&dg_msg,sizeof(dg_msg)))==-1)
      if (errno != EINTR) break; else continue;
    switch (dg_msg.msg_type) {
    case DEATH: break;
    case DCINIT: initialise_client = who; break;
    case DASCMD: dist_DAScmd(&dg_msg, who); break;
    case DCTOKEN:
      DG_rows_requested = dg_msg.u.n_rows;
      if (!dg_msg.u.n_rows) break; /* null token */
      assert(holding_token == 0);
      holding_token = 1;
      while (Reply(who, &my_pid, sizeof(pid_t))==-1 && errno==EINTR);
      break;
    case DCDATA: case DCDASCMD: case TSTAMP:
      msg(MSG_WARN,"Invalid message received by DG of type %d",dg_msg.msg_type);
      break;
    default:
      /* DG_other responsible for replying */
      DG_other((unsigned char *)&dg_msg, who);
      break;
    }
    if (initialise_client && !minf_row) {
      init_client(initialise_client);
      initialise_client = 0;
    }
    if (holding_token) {
      if (dq_DAScmd(&dasc)) {
	if (dist_DCexec(&dasc)) oper_loop = 0;
      } else if (dbr_info.tm_started) {
	if (DG_rows_requested > dbr_info.max_rows || !DG_rows_requested)
	  DG_rows_requested = dbr_info.max_rows;
	if (adjust_rows) {
	  assert(adjust_rows<dbr_info.nrowminf);
	  /* nearest boundary < DG_rows_requested, plus adjust_rows */
	  DG_rows_requested = DG_rows_requested - (DG_rows_requested%dbr_info.nrowminf) + adjust_rows;
	  adjust_rows=0;
	}
	i = dg_delay; j=1;
	while (j!=0 && !breaksignal) j=delay(i);
	if (breaksignal) break;
	sigprocmask(SIG_BLOCK,&sigs,0);	alarm(0);
	DG_get_data(DG_rows_requested);
      } else if (!DG_rows_requested) DG_s_data(0,0,0,0);
      sigprocmask(SIG_UNBLOCK,&sigs,0);		
    }				/* if holding_token */
  }				/* while */

  if (breaksignal) msg(MSG,"caught signal %d: ring shutdown",breaksignal);
  else if (who==-1) msg(MSG_WARN,"error receiving");
  return 0;
}

/* Called from DG_get_data(). */
void DG_s_data(token_type n_rows, unsigned char *data, token_type n_rows1, unsigned char *data1) {
  assert(holding_token);
  if (!data1) n_rows1=0;
  assert( (n_rows+n_rows1) <= DG_rows_requested);
  dr_forward(DCDATA, n_rows, data, n_rows1, data1);
  if (dbr_info.nrowminf > 1)
    minf_row = (minf_row + n_rows + n_rows1) % dbr_info.nrowminf;
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
  assert(!minf_row);
  dasc.type = type;
  dasc.val = val;
  if (dist_DCexec(&dasc)) oper_loop = 0;
}
