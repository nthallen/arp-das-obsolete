/*
 dg.c defines the routines for the distributor portion of the
 data generator. These are the routines common to all DG's.
 $Log$
 * Revision 1.12  1992/09/21  16:05:39  eil
 * extracted DG_init_options into it's own file dg_options.c
 *
 * Revision 1.11  1992/09/02  20:16:37  eil
 * fixing code to handle correct BREAKing
 *
 * Revision 1.10  1992/08/27  18:55:53  eil
 * changed initialisation Reply to DAS_OK, minfrow, dbr_info.
 * also added appropriate msgs.
 *
 * Revision 1.9  1992/08/21  19:46:29  eil
 * fixed bug in DG_s_data, setting minfrow
 *
 * Revision 1.8  1992/08/07  18:10:04  nort
 * Added code to block redundant TM START commands.
 *
 * Revision 1.7  1992/07/17  19:49:14  eil
 * dg will send a TM_START if started by a certain number of clients initialising.
 *
 * Revision 1.6  1992/07/16  14:49:31  eil
 * general code update
 *
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
#include <unistd.h>
#include <limits.h>
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/sendmx.h>
#include <globmsg.h>
#include <eillib.h>
#include <dbr.h>

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
static unsigned int oper_loop=1;
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
  struct _mxfer_entry mlist[3];
  msg_hdr_type rv = DAS_OK;

  _setmx(&mlist[0],&rv, sizeof(msg_hdr_type));
  _setmx(&mlist[1],&minf_row, sizeof(token_type));
  if (!dbr_info.next_tid) {
    dbr_info.next_tid = getpid();
    holding_token=1;
  }
  _setmx(&mlist[2],&dbr_info,sizeof(dbr_info));
  if (!(Replymx(who, 3, mlist))) {
    dbr_info.next_tid = who;
    if (minf_row) adjust_rows = dbr_info.nrowminf-minf_row;
    if (++clients_inited == start_at_clients && !dbr_info.tm_started)
	DG_s_dascmd(DCT_TM,DCV_TM_START);
    msg(MSG,"task %d is my ring (node %d) client",who, getnid());
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
  if (rval != dbr_info.next_tid) {
    msg(MSG,"my neighbor task %d bowed out from node %d ring",dbr_info.next_tid, getnid());
    dbr_info.next_tid = rval;
  }
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
	  if (dbr_info.tm_started) is_dr = 2;
	  else is_dr = 1;
	  break;
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
  switch (is_dr) {
    case 1:
      if (q_DAScmd(msg->u.dasc.type, msg->u.dasc.val)) rep_msg = DAS_BUSY;
    case 2:
      Reply(who, &rep_msg, 1);
      break;
    case 0:
      DG_other((unsigned char *)msg, who);
      break;
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
    } else if (dasc->val == DCV_TM_END)
      dbr_info.tm_started = 0;
  }
  DG_DASCmd(dasc->type, dasc->val);
  dr_forward(DCDASCMD, 0, dasc, 0,0);
  return (dasc->type == DCT_QUIT && dasc->val == DCV_QUIT);
}

void DG_exitfunction(void) {
    if (!dbr_info.next_tid) return; /* no clients */
    if (oper_loop) {
	q_DAScmd(DCT_QUIT,DCV_QUIT);
	DG_operate();
    }
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

  /* misc initialisations */
  BREAK_SETUP;
  my_pid=getpid();
  /* initialize DAScmd queue: */
  dasq.next = dasq.ncmds = 0;
  return(rv);
}

int DG_operate(void) {
  int who;
  dascmd_type dasc;

  do {
      BREAK_PROTECT;
      memset(&dg_msg, DEATH, sizeof(dg_msg));
      do who = Receive(ANY_TASK, &dg_msg, sizeof(dg_msg)); while (who==-1);
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
	  /* DG_other responsible for replying */
          DG_other((unsigned char *)&dg_msg, who);
          break;
      }
    if (holding_token) {
	if (dq_DAScmd(&dasc)) {
          if (dist_DCexec(&dasc)) oper_loop = 0;
        } else if (dbr_info.tm_started) {
          if (DG_rows_requested == 0 || DG_rows_requested > dbr_info.max_rows)
            DG_rows_requested = dbr_info.max_rows;
	  if (adjust_rows) {
	    assert(adjust_rows<dbr_info.nrowminf);
	    /* nearest boundary < DG_rows_requested, plus adjust_rows */
	    DG_rows_requested = DG_rows_requested - (DG_rows_requested%dbr_info.nrowminf) + adjust_rows;
	    adjust_rows=0;
	  }
          DG_get_data(DG_rows_requested);
	}
    } /* if holding_token */
    BREAK_ALLOW;
  }  while (oper_loop);

  BREAK_ALLOW;
  return 0;
}


/* Called from DG_get_data(). */
void DG_s_data(token_type n_rows, unsigned char *data, token_type n_rows1, unsigned char *data1) {
  assert(holding_token);
  if (n_rows) {
    if (!data1) n_rows1=0;
    assert( (n_rows+n_rows1) <= DG_rows_requested);
    dr_forward(DCDATA, n_rows, data, n_rows1, data1);
    if (dbr_info.nrowminf > 1)
	minf_row = (minf_row + n_rows + n_rows1) % dbr_info.nrowminf;
  }
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
  if (dist_DCexec(&dasc))
    oper_loop = 0;
}


