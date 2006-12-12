/* 
dg.c defines the routines for the distributor portion of the
 data generator. These are the routines common to all DG's.
 $Log$
 * Revision 2.1  1994/12/01  21:07:19  eil
 * dfs
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
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#ifdef __QNX__
#include <i86.h>
#endif

#include "globmsg.h"
#include "eillib.h"
#include "das.h"
#include "dg.h"

/* Globals */
topology_type dg_topology = RING;

/* Statics */
static token_type DG_rows_requested = 0; /* private for now */
static int dg_name_id = 0;
static pid_t initialise_client = 0;
static pid_t my_pid = 0;
static pid_t dg_next_tid = 0;
static pid_t star_bow_out = 0;
static int dg_tm_started = 0;
static int clients_inited = 0;
static int start_at_clients = 0;
static int dg_delay = 0;
static token_type adjust_rows = 0;
static unsigned int nrowminf;
static int oper_loop;
static unsigned int minf_row;
static struct {
  dascmd_type q[DASCQSIZE];
  unsigned int next;
  unsigned int ncmds;
} dasq;

#define ADJUST_ROWS adjust_rows=dbr_info.nrowminf-minf_row
#define DG_INFO \
{ \
    dbr_info.tm_started = dg_tm_started; \
    dbr_info.next_tid = dg_next_tid; \
}

/* Determines if DAScmd is valid. Queues for execution when appropriate */
static reply_type dist_DAScmd(UBYTE1 type, UBYTE1 val) {
  reply_type rep_msg = DAS_UNKN;

  switch (type) {
  case DCT_QUIT:
    if (val == DCV_QUIT) {
      if (dg_next_tid == 0) oper_loop = 0;
      rep_msg = DAS_OK;
    }
    break;
  case DCT_TM:
    switch (val) {
    case DCV_TM_END: start_at_clients = -1;
    case DCV_TM_START:
    case DCV_TM_CLEAR: 
    case DCV_TM_SUSLOG:
    case DCV_TM_RESLOG: 
      rep_msg = DAS_OK;
      break;
    default: break;
    }
    break;
  default: break;
  }
  if (rep_msg==DAS_OK)
    if (q_DAScmd(type, val)) rep_msg = DAS_BUSY;
    else
      if (!dg_tm_started && (val==DCV_TM_START || val==DCV_QUIT))
	if (!DG_IS_STAR) dg_tok = 1;
  return(rep_msg);
}

/* Called at exit(). Happens at MSG_EXIT_ABNORM or after DG_operate returns */
void DG_exitfunction(void)
{
  static unsigned char once;
  if (once++) _exit(0);
  breakfunction(-1); /* set ignore breaks */
  if (dg_next_tid || DG_IS_BUS)
    if (oper_loop) {
      /* if tm then QUIT at data rate, else obtains token */
      dist_DAScmd(DCT_QUIT,DCV_QUIT);
      DG_operate();
    }
  qnx_name_d(0,dg_name_id);
  msg(MSG,"task %d: Data Generator operations completed",getpid());
  qnx_ipc_end();
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
    ADJUST_ROWS;
    return(0);
  }
  dasc->type = dasq.q[dasq.next].type;
  dasc->val = dasq.q[dasq.next].val;
  dasq.next++;
  if (dasq.next == DASCQSIZE) dasq.next = 0;
  dasq.ncmds--;
  return(1);
}

static int data_rdy_trig() {
  pid_t proxy;
  dfs_msg_type nd = {DCDATA, 0};

  if (!DG_IS_STAR) return -1;
  if (!dg_next_tid) return -1;
  if (!oper_loop) return -1;
  if (dg_tok) return -1;
  /* attach a null data proxy to client */
  proxy = qnx_proxy_a(dg_next_tid, (char *)&nd, MSG_TOKEN_SZ, -1);
  if (proxy != -1) {
    while (qnx_trig(proxy) == -1)
      if (errno == EINTR) continue;
      else msg(MSG_EXIT_ABNORM,"can't trigger proxy %d",proxy);
  } else msg(MSG_FATAL,"can't attach proxy");
  return 0;
}

/* DG initialises client; returns 0 on success, -1 on error */
static int init_client(pid_t who) {
  struct _mxfer_entry mlist[2];
  msg_hdr_type rv = DAS_OK;
  pid_t old_next_tid;

  old_next_tid = DG_IS_STAR ? 0 : dg_next_tid;
  dbr_info.tm_started = dg_tm_started;

  if (DG_IS_BUS) {
    while (!breaksignal)
      if (fd_write(STDOUT_FILENO,(char *)&dbr_info,DBR_INFO_SZ) == -1)
	if (errno == EINTR) continue;
	else return -1;
      else break;
  } else {

    /* set return code to OK */
    setmx(&mlist[0],&rv, MSG_HDR_SZ);
  
    if (DG_IS_STAR) dbr_info.next_tid = getpid();
    else if (!dg_next_tid) dbr_info.next_tid = getpid();

    /* set up reply */
    setmx(&mlist[1],&dbr_info,DBR_INFO_SZ);
    while (qnx_repmx(who, 2, mlist)==-1)
      if (errno==EINTR) continue;
      else {
	msg(MSG_WARN,"error replying to task %d",who);
	dg_next_tid = old_next_tid;
	return -1;
      }
  }

  dg_next_tid = who;
  if (dg_tm_started) {
    if (old_next_tid == 0)
      if (!DG_IS_STAR) dg_tok = 1;
  } else
    if (++clients_inited == start_at_clients) dist_DAScmd(DCT_TM,DCV_TM_START);

  if (!DG_IS_BUS)
    msg(MSG,"task %d is my %s client",who, DG_IS_RING ? "ring" : "star");

  if (DG_IS_STAR) data_rdy_trig();
  
  return 0;
}

/*
  DG_init() performs initializations:
  Performs sanity checks on dbr_info.
  Initializes remainder of the dbr_info structure.
  Exits on an error we can't continue with.
  Returns zero on success.
*/
int DG_init(int s, int del, topology_type top) {
  static int one;

  DFS_init(top);
  breakfunction(0);
  if (one++) {
    oper_loop=1;
    return(0);
  }
  dg_topology = top;
  
  /* register data generator exit function */
  if (atexit(DG_exitfunction))
    msg(MSG_EXIT_ABNORM,"Can't register DG exit function");

  /* dbr_info; tm info has been determined; at compile time or DG_dac_in() */
  dg_tm_started = 0;
  if (tmi(nbrow)) {
    dbr_info.max_rows = (MAX_BUF_SIZE-MSG_HDR_SZ-TOKEN_SZ)/tmi(nbrow);
    if (dbr_info.max_rows >= UBYTE1_MAX)
      dbr_info.max_rows = UBYTE1_MAX-1;
    dbr_info.nrowminf = tmi(nbminf)/tmi(nbrow);
  }

  /* tstamp remains undefined until TM starts. */
  start_at_clients = s;
  dg_delay = (del < 1) ? 0 : del;
  clients_inited=0;
  if (tmi(nbrow)) dfs_msg_size = dbr_info.max_rows * tmi(nbrow) + DFS_MSG_SZ;
  if (dfs_msg == NULL && dfs_msg_size > 0)
    if ( (dfs_msg=malloc(dfs_msg_size)) == NULL)
      msg(MSG_FATAL,"Can't allocate %d bytes",dfs_msg_size);

  /* misc initialisations */
  my_pid=getpid();
  switch (dg_topology) {
  case STAR:
  case RING: /* attach name */
    if ((dg_name_id=qnx_name_a(getnid(), DG_IS_STAR ?
		   LOCAL_SYMNAME(DB_NAME) : GLOBAL_SYMNAME(DG_NAME)))==-1)
      msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s", DG_IS_STAR ?
          LOCAL_SYMNAME(DB_NAME) : GLOBAL_SYMNAME(DG_NAME));
    break;
  case BUS:
    if (init_client(0)==-1) msg(MSG_FATAL,"Can't pipe out");
    break;
  }
  
  /* initialize DAScmd queue: */
  dasq.next = dasq.ncmds = 0;

  if (!start_at_clients)
    dist_DAScmd(DCT_TM,DCV_TM_START);

  DG_rows_requested=dbr_info.max_rows;

  if (breaksignal)
    msg(MSG_EXIT_NORM,"caught signal %d: shutdown",breaksignal);
  oper_loop = 1;
  block_sigs();
  return 0;
}

/* dr_forward() forwards to the next tid. */
static void dr_forward(msg_hdr_type hdr, token_type n_rows,
  void *other, token_type n_rows1, void *other1)
{
  static pid_t rval;
  static struct _mxfer_entry slist[4];
  static struct _mxfer_entry rlist;
  static int scount;
  static token_type tmp;
  static msg_hdr_type header;

  if (!dg_tok || (!dg_next_tid && !DG_IS_BUS)) return;

  if (hdr) {
    setmx(&rlist,&rval,sizeof(pid_t));
    header = hdr;
    setmx(&slist[0],&header,MSG_HDR_SZ);
    scount=1;
    switch(hdr) {
    case DCDATA:
      tmp=n_rows;
      if (other1 && n_rows1) tmp+=n_rows1;
      setmx(&slist[1],&tmp,TOKEN_SZ);
      scount = 2;
      if (other) {
	setmx(&slist[2],other,n_rows*tmi(nbrow));
	scount++;
      }
      if (other1 && n_rows1) {
	setmx(&slist[3],other1,n_rows1*tmi(nbrow));
	scount++;
      }
      break;
    case TSTAMP:
      setmx(&slist[1],other,TSTAMP_SZ);
      scount = 2;
      break;
    case DCDASCMD:
      setmx(&slist[1],other,DASCMD_SZ);
      scount = 2;
      break;
      /* the following would be a programmatic error */
    default: assert(1); return;
    }
  }

  if (scount)
    while (!breaksignal) {
      if (DG_IS_RING)
	dg_tok=qnx_sndmx(dg_next_tid, scount, 1, slist, &rlist);
      else 
	if (DG_IS_BUS)
	  dg_tok=fd_wrmx(STDOUT_FILENO, scount, slist) > 0 ? 0 : -1;
	else
	  dg_tok=qnx_repmx(dg_next_tid, scount, slist);
      if (dg_tok==-1)
	if (errno==EINTR) continue;
	else {
	  oper_loop=0;
	  msg(MSG_WARN,"error forwarding to %s %d",
	      DG_IS_BUS ? "file descriptor" : "task", 
	      !dg_next_tid ? STDOUT_FILENO : dg_next_tid);
	  dg_next_tid = 0;
	  break;
	}
      else break;
    }

  if (dg_tok) return;
  scount = 0;
  switch (dg_topology) {
  case BUS: 
    if (!start_at_clients) dg_tok = 1;
    break;
  case RING:
    if (rval != dg_next_tid) {
      msg(MSG,"my ring neighbor task %d bowed out",dg_next_tid);
      dg_next_tid = rval;
    }
    break;
  case STAR:
    if (star_bow_out == dg_next_tid) {
      dg_next_tid = 0;
      star_bow_out = 0;
    }
    if (oper_loop!=2) data_rdy_trig();
    break;
  }
}

/* Executes DAScmds and forwards. Returns TRUE if the command is QUIT. */
static int dist_DCexec(dascmd_type *dasc) {
  int xtra_tm = 0;
  DG_INFO;
  if (dasc->type == DCT_TM) {
    xtra_tm = 1;
    if (dasc->val == DCV_TM_START)
      dg_tm_started = 1;
    else if (dasc->val == DCV_TM_END)
      dg_tm_started = 0;
    else xtra_tm = 0;
  }
  if (xtra_tm && dbr_info.tm_started != dg_tm_started) xtra_tm=0;
  if (!xtra_tm) {
    block_sigs();
    DG_DASCmd(dasc->type, dasc->val);
    unblock_sigs();
  }
  /* flag myself trying to send QUIT */
  if (dasc->type == DCT_QUIT && dasc->val == DCV_QUIT && oper_loop) 
    oper_loop=2;
  dr_forward(DCDASCMD, 0, dasc, 0,0);
  if (!dg_tok) { /* successful forward */
    if (!dg_tm_started) dg_tok = 0;
    if (dasc->type == DCT_QUIT && dasc->val == DCV_QUIT) {
      oper_loop = 0;
      dg_next_tid=0;
      return 1;
    }
  }
  return 0;
}

int DG_operate(void) {
  if (!tmi(nbrow))
    msg(MSG_FATAL,"Programmatic Error: initialise before operate");
  unblock_sigs();
  while (oper_loop && !breaksignal) {
    dfs_who = 0;
    if (!dg_tok) {
      dfs_msg->msg_hdr = DFS_default_hdr;
      if ((dfs_who = DFS_rec(dg_topology)) < 1) {
	if (dfs_who == 0) {
	  msg(MSG,"end of input");
	  oper_loop = 0;
	  break;
	} else
	  if (errno == EINTR) continue;
	  else break;
      }
    }
    DG_process_msg();
  }
  if (breaksignal) msg(MSG,"caught signal %d: shutdown",breaksignal);
  else if (dfs_who==-1) {
    oper_loop = 0;
    msg(MSG_WARN,"error receiving");
    return -1;
  }
  block_sigs();
  return 0;
}

reply_type DG_process_msg(void) {
  dascmd_type dasc;
  int i,j;
  reply_type rv = DAS_OK;
  int null_token = 0;

  DG_INFO;
  if (dfs_who)
    switch (dfs_msg->msg_hdr) {
    case DEATH: break;		/* for msgs when clients are signal blocked */
    case DCINIT:
      if (initialise_client) {
	rv=DAS_BUSY;
	if (!DG_IS_BUS)
	  while (qnx_rep(dfs_who,&rv,REPLY_SZ)==-1 && errno==EINTR);
      } else initialise_client=dfs_who;
      break;
    case DCTOKEN:
      if (DG_IS_BUS) break;
      if (dfs_msg->u.n_rows == 0) 
	switch (dg_topology) {
	case RING:
	  null_token = 1;
	  while (qnx_proxy_d(dfs_who)==-1 && errno == EINTR);
	  break;
	case STAR:
	  msg(MSG,"my star client task %d bowed out",dfs_who);
	  dg_next_tid = dfs_who;
	  dg_tok = 1;
	  DG_rows_requested = 0;
	  star_bow_out = dfs_who;
	}
      else {
	if (DG_IS_RING)
	  while (qnx_rep(dfs_who,&my_pid,sizeof(pid_t))==-1 && errno==EINTR);
	/* check that its not a fake token (RING) */
	if (dfs_msg->u.n_rows < UBYTE1_MAX) {
	  DG_rows_requested = dfs_msg->u.n_rows;
	  assert(dg_tok == 0);
	  if (DG_IS_STAR) {
	    dg_next_tid = dfs_who;
	    dg_tok = 1;
	  } else if (dg_tm_started) dg_tok = 1;
	}
      }
      break;
    case DASCMD:
      rv = dist_DAScmd(dfs_msg->u.dasc.type, dfs_msg->u.dasc.val);
      /* DG_other for UNKN reply here ? */
      switch (dg_topology) {
      case RING: case STAR: 
	while (qnx_rep(dfs_who, &rv, REPLY_SZ)==-1 && errno == EINTR);
	break;
      case BUS: break;
      }
      break;
    default:
      block_sigs();
      rv = DG_other((unsigned char *)dfs_msg, dfs_who);
      unblock_sigs();
      if (rv != DAS_NO_REPLY)
	switch (dg_topology) {
	case RING: case STAR:
	  while (qnx_rep(dfs_who, &rv, REPLY_SZ)==-1 && errno==EINTR);
	  break;
	case BUS: break;
	}
      break;
    }

  DG_INFO;
  if (initialise_client)
    if (!minf_row) {
     init_client(initialise_client);
      initialise_client = 0;
    } else ADJUST_ROWS;
  if (dg_tok) dr_forward(0,0,0,0,0);
  if (dg_tok) {
    if (dq_DAScmd(&dasc)) {
      if (dist_DCexec(&dasc)) oper_loop = 0;
    }
    else if (!dg_tm_started || DG_rows_requested == 0) {
      /* no data available packets; needed */
      dr_forward(DCDATA,UBYTE1_MAX,0,0,0);
    }
    else {
      if (DG_rows_requested > dbr_info.max_rows)
        DG_rows_requested = dbr_info.max_rows;
      i = dg_delay; j=1;
      while (j!=0 && !breaksignal) j=delay(i);
      block_sigs();
      DG_INFO;
      if (adjust_rows > DG_rows_requested) adjust_rows = 1;
      DG_get_data(adjust_rows ? adjust_rows : DG_IS_STAR ? 
		  DG_rows_requested - (DG_rows_requested % dbr_info.nrowminf) :
		  DG_rows_requested);
      adjust_rows=0;
    }
  } else if (!dg_tm_started && null_token) {
    /* temporary insanity */
    if (!DG_IS_STAR) {
      dg_tok = 1;
      dr_forward(DCDATA,0,0,0,0);
    }
    dg_tok = 0;
  }

  unblock_sigs();
  return(rv);
}

/* Called from DG_get_data(). */
void DG_s_data(token_type n_rows, unsigned char *data, token_type n_rows1,\
	       unsigned char *data1) {
  unblock_sigs();
  assert(dg_tok);
  assert((n_rows + n_rows1) <= DG_rows_requested);
  /* no data available */
  if (n_rows==0) dr_forward(DCDATA, UBYTE1_MAX, 0, 0, 0);
  else dr_forward(DCDATA, n_rows, data, n_rows1, data1);
  minf_row = (minf_row + n_rows + n_rows1) % dbr_info.nrowminf;
  block_sigs();
}

void DG_s_tstamp(tstamp_type *tstamp) {
  unblock_sigs();
  assert(dg_tok);
  assert(!minf_row);
  dbr_info.t_stmp = *tstamp;
  dr_forward(TSTAMP, 0, tstamp, 0,0);
  block_sigs();
}

void DG_s_dascmd(unsigned char type, unsigned char val) {
/*  dascmd_type dasc;*/
  unblock_sigs();
/*  assert(dg_tok);*/
  dist_DAScmd(type, val);
/*  assert(!minf_row);
  dasc.type = type;
  dasc.val = val;
  if (dist_DCexec(&dasc)) oper_loop = 0;
*/
  block_sigs();
}
