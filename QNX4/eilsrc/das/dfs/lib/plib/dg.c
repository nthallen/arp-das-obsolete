/*
 dg.c defines the routines for the distributor portion of the
 data generator. These are the routines common to all DG's.
 $Log$
 * Revision 1.14  1993/08/24  20:09:34  eil
 * Modified and ported to QNX 4 4/23/92 by Eil.
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
#ifdef __QNX__
#include <i86.h>
#include <sys/psinfo.h>
#endif
#include <sys/types.h>
#include <globmsg.h>
#include <eillib.h>
#include <dg.h>
#include <das.h>

#define DASCQSIZE 5

#ifndef __QNX__
#define sigprocmask(x,y,z);
sigemptyset(x);
sigaddset(x,y);
sigaddset(x,y);
#endif

/* Globals */
token_type DG_rows_requested = 0;

/* Statics */
static pid_t initialise_client = 0;
static int clients_inited = 0;
static int no_token = 0;
static int start_at_clients = 0;
static int dg_delay = 0;
static unsigned int holding_token = 1;
static token_type adjust_rows = 0;
static unsigned int nrowminf;
static unsigned int oper_loop;
static unsigned int minf_row;
static int my_pid;
static struct {
  struct {
    unsigned char type;
    unsigned char val;
  } q[DASCQSIZE];
  unsigned int next;
  unsigned int ncmds;
} dasq;

void DG_exitfunction(void) {
int proxy;
struct {
	hdr_type h;
	dascmd_type d;
} q = { {DASCMD, 0}, DCT_QUIT, DCV_QUIT};
	if (my_ipc == IPC_PIPE) fd_block(STDIN_FILENO);
    if (dbr_info.next_tid)  /* if there are clients */
	if (oper_loop) {
	    breakfunction(0);		
		/* Trigger myself a Quit */
	    proxy = proxy_attach(0,&q,sizeof(q),-1,my_ipc);
	    if (proxy!=-1)
			while(trig(proxy,my_ipc,&q,sizeof(q))==-1 && errno==EINTR);
	    DG_operate();
	}
    msg(MSG,"task %d: DG operations completed",getpid());    
}

/*
	DG_init() performs initializations:
    Performs sanity checks on dbr_info.
    Initializes remainder of the dbr_info structure.
    Exits on an error we can't continue with.
    Returns zero on success.
*/
int DG_init(int s, int del, module_type dg_type) {

/* attach name */
if (qnx_name_attach(getnid(),DG_ONLY(dg_type) == DSG ?
		LOCAL_SYMNAME(DB_NAME) : GLOBAL_SYMNAME(DG_NAME))==-1)
	msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",dg_type == DSG ?
		LOCAL_SYMNAME(DB_NAME) : GLOBAL_SYMNAME(DG_NAME));

sigemptyset(&sigs);
sigaddset(&sigs,SIGINT);
sigaddset(&sigs,SIGTERM);

if (DG_rows_requested!=0) no_token=1;

/* register data generator exit function */
if (atexit(DG_exitfunction))
	msg(MSG_EXIT_ABNORM,"Can't register DG exit function");

/*
   initialize the remainder of dbr_info. tm info should already
   have been determined, either at compile time or by calling DG_dac_in().
*/
dbr_info.tm_started = 0;
dbr_info.next_tid = 0;
if (tmi(nbrow)) {
	dbr_info.max_rows = (unsigned int)((MAX_BUF_SIZE-sizeof(msg_hdr_type)-sizeof(token_type))/tmi(nbrow));
	dbr_info.nrowminf = tmi(nbminf) / tmi(nbrow);
}
dbr_info.mod_type = dg_type;    
/* tstamp remains undefined until TM starts. */
start_at_clients = s;
dg_delay = (del < 1) ? 0 : del;
clients_inited=0;
dfs_msg_size = dbr_info.max_rows * tmi(nbrow) + sizeof(dfs_msg_type);
if ( (dfs_msg=malloc(dfs_msg_size)) == NULL)
	msg(MSG_FATAL,"Can't allocate %d bytes",dfs_msg_size);  	

/* misc initialisations */
my_pid=getpid();
my_ipc=IPC_ONLY(dbr_info.mod_type);
switch (DG_ONLY(dbr_info.mod_type)) {
	case DSG: holding_token=0;
	case DRG:
	  if ( (pflags(~0,_PPF_PRIORITY_REC,0,0,my_ipc)) == -1)
	    msg(MSG_EXIT_ABNORM,"Can't set receiving priority order");
	  pflags(~0,_PPF_SIGCATCH,0,0,my_ipc);
	  break;
}

if (no_token) inunblock(my_ipc);
msg_size=1;
if (my_ipc == IPC_PIPE) fd_write(STDOUT_FILENO,&dbr_info,sizeof(dbr_info));
  
/* initialize DAScmd queue: */
dasq.next = dasq.ncmds = 0;
oper_loop = 1;
alarmfunction(0);
if (!start_at_clients) q_DAScmd(DCT_TM,DCV_TM_START);  	
return 0;
}

/*
   dist_DCexec() executes dbr DAScmds and forwards them on the ring.
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

/*
   dist_DAScmd() determines whether the DAScmd is a dbr DAScmd or not.
   If it is, it queues it for execution when appropriate. Otherwise
   it call DG_other() immediately.
*/
static reply_type dist_DAScmd(dfs_msg_type *msg, pid_t who) {
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
		if (q_DAScmd(msg->u.dasc.type, msg->u.dasc.val)) rep_msg = DAS_BUSY;
		break;
    case 2: rep_msg = DAS_OK; break;
    case 0: rep_msg = DAS_UNKN; break;
}
return(rep_msg);
}

/* dg initialises client and tells it what kind of client it is */
static int init_client(pid_t who) {
  struct _mxfer_entry mlist[2];
  hdr_type rv = { DAS_OK, 0 };

  if (my_ipc == IPC_PIPE) {
  	msg_size=1;
  	return(0);
  }

  /* set return code to OK */
  rv.fromtid = my_pid;
  setmx(&mlist[0],&rv, sizeof(hdr_type));
  
  if (!dbr_info.next_tid) {
  	if (dbr_info.tm_started && DG_ONLY(dbr_info.mod_type) != DSG)
		holding_token = 1;  	
	dbr_info.next_tid = getpid();
  }
  if (DG_ONLY(dbr_info.mod_type) == DSG) dbr_info.next_tid = getpid();

  /* set up reply */
  setmx(&mlist[1],&dbr_info,sizeof(dbr_info));
  while (repmx(who, 2, mlist, my_ipc)==-1)
  	if (errno==EINTR) continue;
  	else {
		msg(MSG_WARN,"error replying to task %d",who);
		rv.msg_type = DAS_UNKN;
	}

  dbr_info.next_tid = who;

  if (minf_row) adjust_rows = dbr_info.nrowminf-minf_row;
  if (++clients_inited == start_at_clients && !dbr_info.tm_started)
	q_DAScmd(DCT_TM,DCV_TM_START);
  msg(MSG,"task %d is my client",who);  
  return 0;
}

/* dr_forward() forwards to the next tid. */
static void dr_forward(msg_hdr_type hdr, token_type n_rows,
		       void *other, token_type n_rows1, void *other1) {
static pid_t rval;
static struct _mxfer_entry slist[5];
static struct _mxfer_entry rlist;
static int scount;
static token_type tmp;
static msg_hdr_type header;
pid_t proxy;
struct {
	hdr_type h;
	token_type t;
} nd = { {DCDATA,0},0};

nd.h.fromtid = my_pid;
if (dbr_info.next_tid == 0) return;
if (scount==0)  {
	setmx(&slist[0],&nd,sizeof(nd));
	scount=1;
}	

if (hdr!=0 && other!=NULL) {
	setmx(&rlist,&rval,sizeof(pid_t));
	header = hdr;
	setmx(&slist[0],&header,sizeof(msg_hdr_type));
	setmx(&slist[1],&my_pid,sizeof(int));
	scount=3;
	switch(hdr) {
		case DCDATA:
			tmp=n_rows;
			if (other1 && n_rows1) tmp+=n_rows1;
			setmx(&slist[2],&tmp,sizeof(token_type));
			setmx(&slist[3],other,n_rows*tmi(nbrow));
			scount=4;
			if (other1 && n_rows1) {
				setmx(&slist[4],other1,n_rows1*tmi(nbrow));
			    scount=5;
			}
			break;
		case TSTAMP:
			setmx(&slist[2],other,sizeof(tstamp_type));
			break;
		case DCDASCMD:
			setmx(&slist[2],other,sizeof(dascmd_type));
			break;
			/* the following would be a programmatic error */
		default: msg(MSG_EXIT_ABNORM,"can't forward msgs with type %d onto ring",hdr);
		return;
	}

	if (dbr_info.mod_type == DSG) {
		/* attach a null data proxy to dbr_info.next_tid */
		if ((proxy=proxy_attach(dbr_info.next_tid,&nd,sizeof(nd),getprio(0)+1,my_ipc))==-1)
			msg(MSG_FATAL,"can't attach proxy");
		/* trigger it and return, keeping token */
		if (trig(proxy,my_ipc,&nd,sizeof(nd))==-1)
			msg(MSG_EXIT_ABNORM,"can't trigger proxy %d",proxy);
		return;
	}
}

sigprocmask(SIG_UNBLOCK,&sigs,0);
while (!breaksignal) {
	if (DG_ONLY(dbr_info.mod_type)==DRG)
		holding_token=sndmx(dbr_info.next_tid, scount, 1, slist, &rlist, my_ipc);
	else
		holding_token=repmx(dbr_info.next_tid, scount, slist, my_ipc);
	if (holding_token==-1)
		if (errno==EINTR) continue;
		else {
			oper_loop=0;
		    msg(MSG_WARN,"error forwarding data to task %d",dbr_info.next_tid);
		    dbr_info.next_tid = 0;
		    break;
		}
	else {
		scount=0;
		break;
	}
}
	
sigprocmask(SIG_BLOCK,&sigs,0);
alarm(0);
if (holding_token) return;
if (my_ipc==IPC_PIPE) return;
if (DG_ONLY(dbr_info.mod_type)==DSG) return;

if (rval != dbr_info.next_tid) {
	msg(MSG,"my ring neighbor task %d bowed out",dbr_info.next_tid);
	dbr_info.next_tid = rval;
}
}

int DG_operate(void) {
pid_t who;
	
breakfunction(0);
while (oper_loop && !breaksignal) {
	dfs_msg->hdr.msg_type = DEATH;
	if ((who=rec(0,dfs_msg,dfs_msg_size,my_ipc,msg_size))==-1) {
		if (errno==EAGAIN || errno==ESRCH) {
			holding_token=1;
			break;
		}
		if (errno != EINTR) break;
		else continue;		
	}
DG_process_msg(who);
}  /* while */

if (breaksignal) msg(MSG,"caught signal %d: shutting down my clients",breaksignal);
else if (who==-1) msg(MSG_WARN,"error receiving");
return 0;
}

int DG_process_msg(pid_t who) {
dascmd_type dasc;
int i,j;
reply_type rv;
int null_token=0;

	switch (dfs_msg->hdr.msg_type) {
		case DEATH: msg_size=1; break;
		case DCINIT: msg_size=sizeof(hdr_type); initialise_client=who; break;
        case DCTOKEN:
			if (DG_ONLY(dbr_info.mod_type)==DSG) dr_forward(0,0,0,0,0);
			else while (rep(who, &my_pid, sizeof(pid_t), my_ipc)==-1 && errno==EINTR);
			msg_size = sizeof(hdr_type) + sizeof(token_type);        
			if (dfs_msg->u.n_rows==0) null_token=1;
			else if (no_token==0) DG_rows_requested = dfs_msg->u.n_rows;
			if (null_token==0 && no_token==0) {
				if (dbr_info.mod_type == DRG) assert(holding_token == 0);
				if (dbr_info.mod_type == DSG) dbr_info.next_tid = who;		  	
				holding_token = 1;
			}
			if (null_token) {
				msg(MSG,"my client task %d bowed out",dfs_msg->hdr.fromtid);
				if (DG_ONLY(dbr_info.mod_type)==DSG &&
					dfs_msg->hdr.fromtid == dbr_info.next_tid) {
					dbr_info.next_tid = 0;
					holding_token = 0;
				}
			}
			break;
        case DASCMD:
			rv = dist_DAScmd(dfs_msg, who);
			msg_size = sizeof(hdr_type) + sizeof(dascmd_type);
       		while (rep( who, &rv, sizeof(reply_type),my_ipc)==-1 && errno==EINTR);
			break;
        default:
          rv = DG_other((unsigned char *)dfs_msg, who, &msg_size);
          if (rv != DAS_NO_REPLY)
		  	while (rep(who, &rv, sizeof(reply_type),my_ipc)==-1 && errno==EINTR);
          break;
    }

	if (initialise_client && !minf_row) {
		init_client(initialise_client);
		initialise_client = 0;
	}
	if (holding_token) {
		if (dq_DAScmd(&dasc)) {
			if (dist_DCexec(&dasc)) oper_loop = 0;
		} else if (null_token) {
				dr_forward(0,0,0,0,0);
				null_token=0;
		}
		else if (dbr_info.tm_started) {
			if (DG_rows_requested > dbr_info.max_rows)
				DG_rows_requested = dbr_info.max_rows;
			if (adjust_rows || dbr_info.mod_type == DSG) {
				assert(adjust_rows < dbr_info.nrowminf);
				/* nearest boundary < DG_rows_requested, plus adjust_rows */
				DG_rows_requested = DG_rows_requested - (DG_rows_requested%dbr_info.nrowminf) + adjust_rows;
				adjust_rows=0;
			}
			i = dg_delay; j=0;
			while (j!=i && !breaksignal) j=delay(i);
/*			if (breaksignal) break;*/
			sigprocmask(SIG_BLOCK,&sigs,0);	alarm(0);
			if (DG_rows_requested) DG_get_data(DG_rows_requested);
		}
	}
	sigprocmask(SIG_UNBLOCK,&sigs,0);				
	return 0;
}

/* Called from DG_get_data(). */
void DG_s_data(token_type n_rows, unsigned char *data, token_type n_rows1, unsigned char *data1) {
int tmp;
  assert(holding_token);
  if (!data1) n_rows1=0;
  tmp = n_rows+n_rows1;
  assert( tmp <= DG_rows_requested);
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
