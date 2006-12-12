/*
dc.c - data clients.
This module contains functions common to all dfs clients.
 $Log$
 * Revision 2.1  1994/12/01  21:07:19  eil
 * dfs
 * Revision 1.4  1993/08/24  20:09:57  eil
Written by DS
Modified 5/23/91 by NTA
Modified 9/26/91 by Eil, changed from ring to buffered ring. (dbr).
Modified extensivley and ported to QNX 4 by Eil 4/22/92.
*/

static char rcsid[] = "";

/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <process.h>
#include <sys/types.h>
#include "globmsg.h"
#include "das.h"
#include "eillib.h"
#include "dc.h"

/* Globals */
token_type DC_data_rows;
topology_type dc_topology = RING;

/* Static variables */
static unsigned int msg_size;
static pid_t ds_tid = 0;
static pid_t my_pid;
static pid_t dc_next_tid = 0;
static pid_t dg_proxy = 0;
static pid_t dc_proxy = 0;
static int dc_tm_started = 0;
static unsigned char bow_out = 0;
static unsigned char oper_loop;
static token_type mf_row;
static msg_token_type dr_tok = {DCTOKEN,0};

#define DC_INFO \
{ \
    dbr_info.next_tid = dc_next_tid; \
    dbr_info.tm_started = dc_tm_started; \
}

/* exit function; Happens at MSG_EXIT_ABNORM or after DC_operate returns */
void DC_exitfunction(void) {
  static unsigned char one;
  if (one++) _exit(0);
  breakfunction(-1);
  if (oper_loop) {
    if (!bow_out) {
      switch (dc_topology) {
      case BUS:
	msg(MSG,"Bus client task %d bowing out, replacing myself with a tee");
	execlp("tee","tee",NULL);
	break;
      case STAR: break;
      case RING:
	if (dg_proxy!=0) {
	  while (qnx_trig(dg_proxy) == -1)
	      if (errno==EINTR) continue;
	      else {
		msg(MSG_WARN,"Can't trigger proxy %d", dg_proxy);
		dg_proxy=0;
		break;
	      }
	}
	break;
      }
      DC_bow_out();
      if (oper_loop) DC_operate();
    }
  }
  if (dc_proxy!=0) qnx_proxy_d(dc_proxy);
  msg(MSG,"task %d: Data Client operations completed",getpid());
  qnx_ipc_end();
}

/* initialises a client into the dfs */
int DC_init(topology_type top, long node) {
  msg_hdr_type dr_init = DCINIT;
  reply_type r = DAS_UNKN;
  struct _mxfer_entry tlist[2];
  struct _mxfer_entry flist[3];
  static int one;
  dfs_msg_type null_data = {DCDATA, 0};

  DFS_init(top);
  breakfunction(0);
  if (one++) {
    oper_loop=1;
    return(0);
  }
  dc_topology=top;
  my_pid=getpid();


  /* register data client exit function */
  if (atexit(DC_exitfunction))
    msg(MSG_EXIT_ABNORM,"Can't register DC exit function");

  /* ds_tid is source of data for a client */
  switch (dc_topology) {
  case STAR: 
    setprio(getpid(),getprio(0)-1);
    /* attach a null data proxy to myself */
    if ((dc_proxy=qnx_proxy_a(0,(char *)&null_data,MSG_TOKEN_SZ,-1))==-1)
      msg(MSG_EXIT_ABNORM,"Can't attach proxy to myself");
  case RING: /* locate name */
    if ((ds_tid=qnx_name_l(node, DC_IS_STAR ?
			   LOCAL_SYMNAME(DB_NAME) : GLOBAL_SYMNAME(DG_NAME), 
			   0, 0)) == -1)
      msg(MSG_EXIT_ABNORM,"Can't find symbolic name for %s on node %d",
	  DC_IS_STAR ? DB_NAME : DG_NAME, node);
    if (ds_tid==my_pid)
      msg(MSG_EXIT_ABNORM,"My data source can't be myself");
    setmx(&tlist[0], &dr_init, MSG_HDR_SZ);
    setmx(&tlist[1], &dc_proxy, sizeof(BYTE2));
    setmx(&flist[0], &r, REPLY_SZ);
    setmx(&flist[1], &dbr_info, DBR_INFO_SZ);
    setmx(&flist[2], &dg_proxy, sizeof(BYTE2));
    while (!breaksignal) {
      if ( !(oper_loop=!qnx_sndmx(ds_tid, 2, 3, &tlist, &flist)) )
	if (errno == EINTR) continue;
	else {
	  ds_tid = -1;
	  msg(MSG_EXIT_ABNORM,"Error sending to data source task %d",ds_tid);
	}
      if (r == DAS_BUSY && !DC_IS_STAR) continue;
      else break;
    }
    break;
  case BUS:
    ds_tid=STDIN_FILENO;
    while (!breaksignal)
      if (fd_read(STDIN_FILENO,(char *)&dbr_info,DBR_INFO_SZ) == -1)
	if (errno == EINTR) continue;
	else msg(MSG_FATAL,"Can't pipe in");
      else {
	oper_loop = 1;
	break;
      }
  }

  dc_next_tid = dbr_info.next_tid;
  dc_tm_started = dbr_info.tm_started;
  if (tmi(nbrow)) dfs_msg_size = dbr_info.max_rows * tmi(nbrow) + DFS_MSG_SZ;
  if (dfs_msg == NULL && dfs_msg_size > 0)
    if ( (dfs_msg=malloc(dfs_msg_size)) == NULL)
      msg(MSG_FATAL,"Can't allocate %d bytes",dfs_msg_size);   

  if (!DC_data_rows) DC_data_rows = dbr_info.max_rows;
  else if (DC_data_rows > dbr_info.max_rows) {
    msg(MSG_WARN,"min data msg size %d > %d allowable, defaulted",
	DC_data_rows,dbr_info.max_rows);
    DC_data_rows=dbr_info.max_rows;
  }
    
  if (breaksignal) 
    msg(MSG_EXIT_NORM,"caught signal %d: exiting",breaksignal);
  block_sigs();
  return 0;
}

/* holding_token() handles all states while we have the token. */
static int holding_token(void) {
  int rcv_buf;
  pid_t dr;
  reply_type rv = REP_MAX + 1;
  void *ptr;
  int full_token = 0;

  if (my_pid==0) dc_tok=0;
  if (!dc_tok) return 0;
  rcv_buf=~dc_next_tid;

  if (dc_next_tid != ds_tid) {
    /* BUS and inner ring clients */
    while (!breaksignal) {
      if (DC_IS_RING)
	dc_tok=qnx_snd(dc_next_tid,(char *)dfs_msg,&rcv_buf,
		       msg_size, sizeof(pid_t));
      else
	dc_tok=(fd_write(STDOUT_FILENO, (char *)dfs_msg, msg_size))
	  >= 0 ? 0 : -1;
      if (dc_tok==-1)
	if (errno==EINTR) continue;
	else break;
      else break;
    }
  } else {
    if (ds_tid<0) {
      dc_tok = 0;
      return 0;
    }
    if (DC_IS_RING) {
      /* if null data for a ring client , dont pass on token */
      if (dfs_msg->msg_hdr == DCDATA && dfs_msg->u.drd.n_rows == 0)
	full_token = 1;
      else {
	if (dc_tm_started)
	  /* data regulation */
	  while (!breaksignal && !DC_data_rows) {
	    if ((dr=qnx_rec(0, dfs_msg, DFS_MSG_SZ ))==-1)
	      if (errno!=EINTR) break;
	      else continue;
	    else 
	      switch (dfs_msg->msg_hdr) {
	      case DCDATA: case TSTAMP: case DCDASCMD: 
		msg(MSG_WARN,"Invalid msg at data regulation stage: type %d",
		    dfs_msg->msg_hdr);
		break;
	      default:
		/* Eventually this may reply with a message unknown */
		rv=DC_other((char *)dfs_msg, dr);
		if (rv!=DAS_NO_REPLY)
		  while (qnx_rep(dr,&rv,REPLY_SZ)==-1 && errno==EINTR);
		break;
	      }
	  }
      }
    }
    if (DC_IS_STAR) ptr = dfs_msg; else ptr = &rcv_buf;
    if (full_token) dr_tok.n_rows = UBYTE1_MAX; /* fake token */
    else dr_tok.n_rows = DC_data_rows;
    while (!breaksignal)
      if ((dc_tok=qnx_snd(dc_next_tid,&dr_tok,ptr,MSG_TOKEN_SZ,
			  ptr==dfs_msg ? dfs_msg_size :
			  sizeof(rcv_buf)))==-1)
	if (errno!=EINTR) break;
	else continue;
      else break;
  }

  switch (dc_tok) {
  case -1:
    /* pass on of token failed */
    if (!breaksignal)
      if (errno==ESRCH)		/* couldn't find neighbor */
	/* If it was the DG, quit. Otherwise send to DG from here on */
	if (dc_next_tid == ds_tid) {
	  oper_loop = 0;	/* no need to bow out */
	  ds_tid = -1;
	}
	else {
	  if (dc_next_tid)
	    msg(MSG,"my ring neighbor task %d bowed out",dc_next_tid);
	  dc_next_tid = ds_tid;
	}
      else
	oper_loop = 0;
    break;
  case 0:			/* success */
    switch (dc_topology) {
    case RING:
      if (rcv_buf != dc_next_tid) {
	dc_next_tid = rcv_buf;
	if (!rcv_buf) {
	  dc_next_tid=ds_tid;
	  dc_tok = dc_next_tid;
	  holding_token();
	}
	msg(MSG,"my new ring neighbor task is %d",dc_next_tid);
      }
      break;
    case STAR:
      if (DC_data_rows == 0 /* bow out */) {
	oper_loop = 0;
	ds_tid = -1;
	dc_next_tid = 0;
      }
      else {
	dfs_who = dc_next_tid;
	DC_process_msg();
      }
      break;
    }
    break;
  }

  return(dc_tok);
}

/* client operation. returns 0 on success */
int DC_operate(void) {
  if (!tmi(nbrow))
    msg(MSG_FATAL,"Programmatic Error: initialise before operate");
  unblock_sigs();
  while (oper_loop && !breaksignal) {
    dfs_who = 0;
    if (!dc_tok && !dg_tok) {
      dfs_msg->msg_hdr = DFS_default_hdr;
      if ((dfs_who = DFS_rec(dc_topology)) < 1)
	if (dfs_who == 0) {
	  msg(MSG,"end of input");
	  /*ds_tid=-1;*/
	  oper_loop=0;
	  break;
	} else
	  if (errno == EINTR) continue;
	  else break;
    }
    DC_process_msg();
  }

  if (breaksignal) msg(MSG,"caught signal %d: disengaging",breaksignal);
  else if (dfs_who==-1) {
    oper_loop = 0;
    msg(MSG_WARN,"error receiving data");
    return -1;
  } else if (dc_tok==-1) {
    oper_loop = 0;
    msg(MSG_WARN,"error forwarding data");
    return -1;
  }
  block_sigs();
  return 0;
}

reply_type DC_process_msg() {
  reply_type rv = DAS_OK;
  int do_other = 0;

  DC_INFO;
  block_sigs();
  if (dfs_who > 0) {
    switch (dfs_msg->msg_hdr) {
    case DCDATA:
      if (DC_IS_STAR) {
	/* data ready kick, null data proxy */
	if (dfs_msg->u.drd.n_rows == 0 && dfs_who == dc_proxy)
	    dc_tok = dfs_who;
	break;
      }
    case TSTAMP: case DCDASCMD:
      switch (dc_topology) {
      case STAR: break; /* reply unknown to anybody who isnt ds_tid */
      case BUS:
	dc_tok = dfs_who;
	break;
      case RING:
	dc_tok = dfs_who;
	if (bow_out && !mf_row) {
	  oper_loop = 0;
	  my_pid=0;
	  if (dc_next_tid != ds_tid) {
	    while (qnx_relay(dfs_who, dc_next_tid, (char *)dfs_msg,
			     dfs_msg_size) == -1 && errno == EINTR);
	    break;
	  }
	}
	while (qnx_rep(dfs_who,&my_pid,sizeof(pid_t)) == -1 && errno == EINTR);
	break;
      }
    }

    switch (dfs_msg->msg_hdr) {
    case DCDATA:
      msg_size = dfs_msg->u.drd.n_rows * tmi(nbrow) + MSG_TOKEN_SZ;
      if (dfs_msg->u.drd.n_rows && dfs_msg->u.drd.n_rows < UBYTE1_MAX) {
	DC_data(&dfs_msg->u.drd);
	mf_row = (mf_row + dfs_msg->u.drd.n_rows) % dbr_info.nrowminf;
      }
      break;
    case TSTAMP:
      if (mf_row)
	msg(MSG_FATAL,"Programmatic Error: timestamp not on MF boundary");
      dbr_info.t_stmp = dfs_msg->u.tst;
      msg_size = MSG_HDR_SZ + TSTAMP_SZ;
      DC_tstamp(&dfs_msg->u.tst);
      break;
    case DCDASCMD:
      {
	int xtra_tm = 0;
	if (mf_row)
	  msg(MSG_FATAL,"Programmatic Error: DC DAS cmd not on MF boundary");
	msg_size = MSG_DASCMD_SZ;
	switch (dfs_msg->u.dasc.type) {
	case DCT_TM:
	  switch (dfs_msg->u.dasc.val) {
	  case DCV_TM_START:
	    dc_tm_started = 1; xtra_tm = 1; break;
	  case DCV_TM_END:
	    dc_tm_started = 0; xtra_tm = 1; break;
	  default: break;
	  }
	  break;
	case DCT_QUIT:
	  switch (dfs_msg->u.dasc.val) {
	  case DCV_QUIT:
	    switch (dc_topology) {
	    case STAR: case RING: 
	      if (dc_next_tid == ds_tid) my_pid = 0;
	    case BUS:
	      oper_loop = 0;
	      break;
	    }
	    break;
	  default: break;
	  }
	default: break;
	}
	if (xtra_tm && dbr_info.tm_started != dc_tm_started) xtra_tm = 0;
	if (!xtra_tm)
	  DC_DASCmd(dfs_msg->u.dasc.type, dfs_msg->u.dasc.val);
	break;
      }
    case DEATH: break;
    default: do_other = 1; break;
    }
  } else if (dg_tok) do_other = 1;

  if (do_other) {
    rv = DC_other((unsigned char *)dfs_msg, dfs_who);
    if (!DC_IS_BUS && rv != DAS_NO_REPLY && dfs_who > 0)
      while (qnx_rep(dfs_who, &rv, REPLY_SZ) && errno == EINTR);
  }

  unblock_sigs();
  holding_token();
  return(rv);
}

/* disengage gracefully from dfs */
int DC_bow_out(void) {
  if (ds_tid < 0) oper_loop = 0;
  if (my_pid == 0) oper_loop = 0;
  if (bow_out) return 0;
  if (DC_IS_STAR) DC_data_rows = 0;
  if (oper_loop) msg(MSG,"client task %d: bowing out",getpid());
  bow_out=1;
  return 0;
}
