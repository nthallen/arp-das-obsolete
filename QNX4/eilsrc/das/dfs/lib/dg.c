/*
 dg.c defines the routines for the distributor portion of the
 data generator. These are the routines common to all DG's.
 Written by NTA April 24, 1991
 Modified Aug 20, 1991 by Eil, to attach data_gen name and register with cmdctrl.
 Modified Aug 27, 1991 by Eil, so some task gets dg as next_tid.
 Modified Sep 26, 1991 by Eil, changing from ring to buffered ring.
 Modified and ported to QNX 4 4/23/92 by Eil.
 $Log$
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
#include <sys/kernel.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/types.h>
#include <sys/sendmx.h>
#include <cmdctrl.h>
#include <globmsg.h>
#include <das_utils.h>
#include <dbr_utils.h>

/* Preprocessor definitions: */
#define ANY_TASK 0
#define DASCQSIZE 5

/* Structures for this particular module */
typedef struct {
  unsigned char msg_type;
  union {
    unsigned char n_rows; /* for token */
    dascmd_type dasc;
  } u;
} dg_msg_type;

/* Globals */
int dg_id;
unsigned short DG_seq_num;

/* Static Variables:
   The DG does not receive data, so it's message buffer needn't
   be sized for data. I expect to send data via sendm which
   doesn't require contiguous buffers. We do, however, need
   to receive other message types that we can predict. It
   may be necessary to require particular applications to
   call read_msg() or read_msgm() to get longer messages.
*/
static dg_msg_type dg_msg;
static unsigned int holding_token = 1;
static unsigned int DG_rows_requested = 0;
static unsigned int nrowminf;
static unsigned int oper_loop;
static unsigned short seq_num;
static pid_t my_pid;
static struct {
  struct {
    unsigned char type;
    unsigned char val;
  } q[DASCQSIZE];
  unsigned int next;
  unsigned int ncmds;
} dasq;


int init_client(int who) {
  unsigned char rv = DAS_OK;
  struct _mxfer_entry mlist[2];

  _setmx(&mlist[0],&rv, 1);
  if (!dbr_info.next_tid) dbr_info.next_tid = getpid();
  _setmx(&mlist[1],&dbr_info,sizeof(dbr_info));
  if (!(Replymx(who, 2, mlist)))
    dbr_info.next_tid = who;
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

/* returns TRUE if queue is non-empty, FALSE if empty */
static int dq_DAScmd(dascmd_type *dasc) {
  if (dasq.ncmds == 0) return 0;
  dasc->type = dasq.q[dasq.next].type;
  dasc->val = dasq.q[dasq.next].val;
  dasq.next++;
  if (dasq.next == DASCQSIZE) dasq.next = 0;
  dasq.ncmds--;
}


/* dr_forward() forwards to the next tid. */
static void dr_forward(unsigned char msg_type, unsigned char n_rows,
		       void *other, unsigned int size) {
  static pid_t rval;
  static unsigned char sval[2];
  static struct _mxfer_entry slist[4];
  static struct _mxfer_entry rlist;
  int scount;

  if (dbr_info.next_tid == 0) return;
  _setmx(&rlist,&rval,sizeof(pid_t));
  _setmx(&slist[0],&msg_type,1);
  _setmx(&slist[1],&DG_seq_num, 2);
  scount=2;
  if (n_rows > 0) {
    _setmx(&slist[2],&n_rows,1);
    scount++;
  }
  if (size > 0) {
    _setmx(&slist[scount],other,size);
    scount++;
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
static void dist_DAScmd(dg_msg_type *msg, int who) {
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
      dbr_info.minf_row = 0;
    } else if (dasc->val == DCV_TM_END)
      dbr_info.tm_started = 0;
  }
  DG_DASCmd(dasc->type, dasc->val);
  dr_forward(DCDASCMD, 0, dasc, sizeof(dascmd_type));
  return (dasc->type == DCT_QUIT && dasc->val == DCV_QUIT);
}

int DG_init_options(int argcc, char **argvv) {
extern char *optarg;
extern int optind, opterr, optopt;
int timing=RT;
char filename[FILENAME_MAX] = {'\0'};
	int c;

    /* error handling intialisation if the client code didnt */
    if (!msg_initialised())
	msg_init(DG_NAME,0,1,0,0,1);

    opterr = 0;
    optind = 0;

    do {
	  c=getopt(argcc,argvv,opt_string);
	  switch (c) {
		case 'p': timing=DT;  break;
		case '?':
		  msg(MSG_EXIT_ABNORM, "Invalid option -%c", optopt);
		default : break;
	  }
	} while (c != -1);
    optind = 0;
    opterr = 1;
    return(DG_init(timing));
}


/* DG_init() performs initializations:
    Performs sanity checks on dbr_info.
    Initializes remainder of the dbr_info structure.
    Exits on an error we can't continue with.
    Returns non-zero if there is a problem, but we can continue.

    Assume ring is faster than the desired rate.
    Every DG is either RT or DT.
    RT means data must be generated at an exact rate.
*/
int DG_init(unsigned char timing) {
  unsigned char rv = 0;
  int cmd_tid;
  ccreg_type reg;  
  int t;
  unsigned char replycode;
  char name[FILENAME_MAX+1];

  /* error handling intialisation if the client code didnt */
  if (!msg_initialised())
    msg_init(DG_NAME,0,1,0,0,1);

  /* attach name */
  if ((dg_id=qnx_name_attach(getnid(),LOCAL_SYMNAME(DG_NAME,name)))==-1)
    msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);  

  if ( (cmd_tid = qnx_name_locate(getnid(), LOCAL_SYMNAME(CMD_CTRL,name),0,0))==-1)
    msg(MSG_WARN,"Can't find %s",name);
  else {
    reg.ccreg_byt  = CCReg_MSG;
    reg.min_dasc = DCT_QUIT;
    reg.max_dasc = DCT_TM;
    reg.min_msg = reg.max_msg = 0;
    reg.how_to_quit = FORWARD_QUIT;
    t = Send( cmd_tid, &reg, &replycode, sizeof(reg), 1 );
	if (t == -1 || replycode !=DAS_OK) {
		/* this is something we can live with */
		msg(MSG_WARN,"Can't communicate with cmdctrl task");
		rv = 1;
	}
  }

  /* initialize the remainder of dbr_info. tm info should already
     have been determined, either at compile time or by calling
	 DG_dac_in().
  */
  dbr_info.tm_started = 0;
  dbr_info.timing = timing;
  dbr_info.mod_type = DG;
  dbr_info.next_tid = 0;
  dbr_info.max_rows = (unsigned int)((MAX_BUF_SIZE - 5)/tmi(nbrow));
  if (dbr_info.max_rows == 0)
    dbr_info.max_rows = 1;
  /* tstamp remains undefined until TM starts. */
  nrowminf = tmi(nbminf) / tmi(nbrow);

  /* initialize DAScmd queue: */
  dasq.next = dasq.ncmds = 0;

  DG_seq_num = 0;
  my_pid=getpid();

  return(rv);
}

int DG_operate(void) {
  int who;
  dascmd_type dasc;

  for (oper_loop = 1; oper_loop != 0; ) {
    do who = Receive(ANY_TASK, &dg_msg, sizeof(dg_msg)); while (who < 0);
    if (dg_msg.msg_type == DCINIT) init_client(who);
    else
      switch (dg_msg.msg_type) {
        case DASCMD:
          dist_DAScmd(&dg_msg, who);
          break;
        case DRTOKEN:
          assert(holding_token == 0);
          DG_rows_requested = dg_msg.u.n_rows;
          holding_token = 1;
          Reply(who, &my_pid, 1);
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
          if (DG_rows_requested == 0 ||
              DG_rows_requested > dbr_info.max_rows)
            DG_rows_requested = dbr_info.max_rows;
          DG_get_data(DG_rows_requested);
	}
    }
  }
  return 0;
}


/* Called from DG_get_data(). */
void DG_s_data(unsigned int n_rows, unsigned char *data) {
  assert(holding_token);
  assert(n_rows != 0);
  assert(n_rows <= DG_rows_requested);
  dr_forward(DCDATA, n_rows, data, n_rows * tmi(nbrow));
  if (nrowminf > 1)
    dbr_info.minf_row = (dbr_info.minf_row + n_rows) % nrowminf;
}


void DG_s_tstamp(tstamp_type *tstamp) {
  assert(holding_token);
  dbr_info.t_stmp = *tstamp;
  dr_forward(TSTAMP, 0, tstamp, sizeof(tstamp_type));
}


void DG_s_dascmd(unsigned char type, unsigned char val) {
  dascmd_type dasc;

  assert(holding_token);
  dasc.type = type;
  dasc.val = val;
  if (dist_DCexec(&dasc))
    oper_loop = 0;
}
