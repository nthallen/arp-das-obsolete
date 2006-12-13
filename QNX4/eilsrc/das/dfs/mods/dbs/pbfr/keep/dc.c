/*
dc.c - data clients.
This module contains functions common to all dbr clients.
 $Log$
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
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <process.h>
#ifdef __QNX__
#include <i86.h>
#include <sys/kernel.h>
#include <sys/proxy.h>
#include <sys/sendmx.h>
#include <sys/name.h>
#include <sys/sched.h>
#include <sys/psinfo.h>
#include <sys/types.h>
#endif
#include <eillib.h>
#include <dc.h>
#include <globmsg.h>

#ifndef __QNX__
#define sigprocmask(x,y,z);
#endif

/* Globals */
token_type DC_data_rows;

/* Static variables */
static pid_t fromtid = 0;
static pid_t got_tok = 0;
static int ds_tid = 0;
static unsigned int msg_size;
static unsigned char bow_out = 0;
static unsigned char oper_loop;
static token_type mf_row;
static pid_t my_pid;
static struct {
    msg_hdr_type msg_type;
    token_type n_rows;
} dr_tok = { DCTOKEN,0 };

/* the exit function during client operation ensuring proper bow-out */
void DC_exitfunction(void) {
if (oper_loop) {
    breakfunction(0);
	if (!bow_out) {
		switch (dbr_info.mod_type) {
#ifdef __QNX__
pid_t pr;
			default:
				holding_token();
				dr_tok.n_rows = 0;
				if (dbr_info.mod_type == DRC) {
					pr=qnx_proxy_attach(ds_tid,(char *)&dr_tok,sizeof(dr_tok),-1);
					while (Trigger(pr)==-1)
						if (errno==EINTR) continue;
						else return;
				} else Send(ds_tid,&dr_tok,&pr,sizeof(dr_tok),sizeof(pr));
				break;
#endif
			case DBC:
				msg(MSG,"task %d: bowing out, replacing myself with a tee");
				execlp("tee","tee",NULL);
				break;
		}
	    DC_bow_out();
	    DC_operate();
	}
}
msg(MSG,"task %d: DC operations completed",getpid());
}

/* initialises a client into the dfs.*/
int DC_init(int client_type, long node) {
#ifdef __QNX__	
msg_hdr_type dr_init = DCINIT;
int pri;
struct _mxfer_entry mlist[2];
#endif

    oper_loop=1;
    bow_out = 0;
    breakfunction(0);
    my_pid=getpid();
#ifdef __QNX__
    sigemptyset(&sigs);
    sigaddset(&sigs,SIGINT);
    sigaddset(&sigs,SIGTERM);
#endif
    alarmfunction(0);

    if (!IS_DC(client_type))  return 1;
    oper_loop=0;

    /* register data client exit function */
	if (atexit(DC_exitfunction))
		msg(MSG_EXIT_ABNORM,"Can't register DC exit function");

    /* ds_tid is source of data for a client, either dg or db or stdin */
    switch (client_type) {
    case DBC:
		oper_loop=1; ds_tid=-1;
		bus_read(STDIN_FILENO,(char *)&dbr_info);
		bus_nonblock(STDIN_FILENO);    
    	break;
#ifdef __QNX__
	case DSC:
		if ((ds_tid=qnx_name_locate(node, LOCAL_SYMNAME(DB_NAME), 0, 0)) == -1)
		    msg(MSG_EXIT_ABNORM,"Can't find symbolic name for %s on node %d",DB_NAME, node);
		pri = getprio(0);
		if (--pri>0) setprio(getpid(),pri);
		break;
	case DRC:
		if ((ds_tid=qnx_name_locate(0, GLOBAL_SYMNAME(DG_NAME), 0, 0)) == -1)
		    msg(MSG_EXIT_ABNORM,"Can't find symbolic name for %s");
		qnx_pflags(~0,_PPF_SIGCATCH,0,0);
		break;
#endif
    }
			    
    if (ds_tid==my_pid) msg(MSG_EXIT_ABNORM,"My data source can't be myself");

#ifdef __QNX__
	switch (client_type) {
	case DRC:
	case DSC:	    
	    /* get dbr_info */
	    _setmx(&mlist[0], &dr_init, sizeof(msg_hdr_type));
	    _setmx(&mlist[1], &dbr_info, sizeof(dbr_info));
		while (!breaksignal)
			if ( !(oper_loop=!Sendmx(ds_tid, 1, 2, &mlist, &mlist)) )
				if (errno != EINTR)
					msg(MSG_EXIT_ABNORM,"Error sending to my data source task %d",ds_tid);
				else continue;
			else break;
		if (!breaksignal)
		    if (dr_init != DAS_OK) {
		    	oper_loop = 0;
				msg(MSG_EXIT_ABNORM,"bad response from data source task %d at registration",ds_tid);
			}
	}
#endif

	dbr_info.mod_type = client_type;    
    /* make space for data transfers */
    dfs_msg_size = dbr_info.max_rows * dbr_info.tm.nbrow + sizeof(msg_hdr_type) + sizeof(dbr_data_type);
    if (dfs_msg_size < sizeof(dfs_msg_type)) dfs_msg_size = sizeof(dfs_msg_type);
	if ( !(dfs_msg = malloc(dfs_msg_size)))
		msg(MSG_EXIT_ABNORM,"Can't allocate %d bytes of memory",dfs_msg_size);
	dfs_msg->msg_type = DEATH;
    if (!DC_data_rows) DC_data_rows = dbr_info.max_rows;
	else if (DC_data_rows > dbr_info.max_rows) {
		msg(MSG_WARN,"ÿmin data msg size %d > %d allowable, defaulted",DC_data_rows,dbr_info.max_rows);
		DC_data_rows=dbr_info.max_rows;
	}
	
	if (dbr_info.mod_type==DSC) {
		/* make a request */
		got_tok = ds_tid;
		holding_token();
	}

	if (breaksignal) msg(MSG_EXIT_NORM,"caught signal %d: exiting",breaksignal);
    return 0;
}

static void replyback(pid_t who, void *r, int sz) {
	if (who < 1) return;
#ifdef __QNX__
	while (Reply(who, r, sz)==-1 && errno==EINTR);
#endif
}

/* holding_token() handles all states while we have the DRing token. */
static int holding_token(void) {
pid_t rcv_buf;

	switch (dbr_info.mod_type) {
		case DBC: bus_write(STDOUT_FILENO,0,(char *)dfs_msg,msg_size); break;
#ifdef __QNX__		
		default:
			if (got_tok < 1 || ds_tid < 1) return 0;
			switch (dfs_msg->msg_type) {
				case DCDATA: case TSTAMP: case DCDASCMD: case DEATH: break;
				default: return 0; /* other message */
			}
			if (dbr_info.next_tid != ds_tid) {
				while (!breaksignal)
					if ( (got_tok=Send(dbr_info.next_tid,dfs_msg,&rcv_buf,msg_size, sizeof(pid_t)))==-1)
						if (errno!=EINTR) break;
						else continue;
					else break;
			} else {
				if (dbr_info.tm_started)
					/* data regulation */
					while (dbr_info.mod_type == DRC && !breaksignal && !DC_data_rows) {
						if (Receive(0, dfs_msg, dfs_msg_size)==-1)
							if (errno!=EINTR) break;
							else continue;
						else switch (dfs_msg->msg_type) {
							case DCDATA: case TSTAMP: case DCDASCMD: 
							    msg(MSG_WARN,"Invalid msg received at data regulation stage: type %d",dfs_msg->msg_type);
								break;
							default:
					            /* Eventually this may reply with a message unknown */
					            DC_other((unsigned char *)dfs_msg, rcv_buf); 
					            break;
						}
					}
				dr_tok.n_rows = DC_data_rows;
				while (!breaksignal)
					if ( (got_tok=Send(dbr_info.next_tid,&dr_tok,&rcv_buf,sizeof(dr_tok),sizeof(rcv_buf)))==-1)
						if (errno!=EINTR) break;
						else continue;
					else break;
			}

			if (breaksignal) return 0;
			/* pass on of token failed */
			if (got_tok == -1) {
				if (errno==ESRCH)  /* couldn't find neighbor */
					/* If it was the DG, quit. Otherwise send to DG from here on */
					if (dbr_info.next_tid == ds_tid) {
					    got_tok = ds_tid;
					    oper_loop = 0; /* no need to bow out */
					    msg(MSG_WARN,"my data source task %d is gone",ds_tid);
					    return -1;
					} else {
					    if (dbr_info.next_tid)
						    msg(MSG,"my ring neighbor task %d bowed out",dbr_info.next_tid);
					    dbr_info.next_tid = ds_tid;
					    got_tok = ds_tid;
					}
			} else if (rcv_buf != dbr_info.next_tid) {
				dbr_info.next_tid = rcv_buf;
				if (!rcv_buf) {
				    dbr_info.next_tid=ds_tid;
				    got_tok = dbr_info.next_tid;
				    holding_token();
				}
				msg(MSG,"my new neighbor task is %d",dbr_info.next_tid);
			}
	}
#endif
  return 0;
}

/* client operation. returns 0 on success */
int DC_operate(void) {
int rtrn_code = 0;  /* Code returned to calling function */
int bower;

while (oper_loop && !breaksignal) {
	bower = 0;
	dfs_msg->msg_type = DEATH;
#ifdef __QNX__
	if (dbr_info.mod_type != DBC) {
		/* receive for data */    	
		if ((got_tok = Receive(fromtid, dfs_msg, dfs_msg_size)) == -1)
			if (errno != EINTR) break; else continue;
		fromtid = 0;
		/* Handle bow_out conditions */
		if (bow_out)
			switch (dbr_info.mod_type) {
				case DRC:
					if (!mf_row)
					    switch (dfs_msg->msg_type) {
						case DCDATA: case DCDASCMD: case TSTAMP: bower = 1;
						    if (dbr_info.next_tid == ds_tid) {
								my_pid=0;
								replyback(got_tok, &my_pid, sizeof(pid_t));
						    } else while (Relay(got_tok, dbr_info.next_tid)==-1 && errno==EINTR);
						default : break;
					    }
					    break;
				case DSC:
					if (DC_data_rows) DC_data_rows=0;
					else bower=1;
			}
    } else
#endif
	if (dbr_info.mod_type==DBC) {
		if ((bower=bus_read(STDIN_FILENO, (char *)dfs_msg))==0) {
			msg(MSG,"reached end of file");
			DC_bow_out();
		}
		if (bower==-1) msg(MSG_EXIT_ABNORM,"error reading from stdin");
		bower=!bower;
	}

    if (!bower)
    	rtrn_code = DC_process_msg(got_tok);
    else  { oper_loop=0; ds_tid=0; }
  } /* while */

 if (breaksignal) msg(MSG,"caught signal %d: disengaging",breaksignal);
 else if (got_tok==-1) {
    oper_loop = 0;
    rtrn_code = -1;
    msg(MSG_WARN,"error getting data");
 }
 return rtrn_code;
}

int DC_process_msg(int who) {
int rtrn_code = 0;
reply_type rv = DAS_OK;
/* switch on data header */
switch (dfs_msg->msg_type) {
	case DEATH: break;
    case DCDATA:
		replyback(got_tok, &my_pid, sizeof(pid_t));
		msg_size = dfs_msg->u.drd.n_rows * tmi(nbrow) + sizeof(msg_hdr_type) + sizeof(token_type);
		if (dfs_msg->u.drd.n_rows) {
		    sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
		    DC_data(&dfs_msg->u.drd);
		    sigprocmask(SIG_UNBLOCK,&sigs,0);
		    mf_row = (mf_row + dfs_msg->u.drd.n_rows) % dbr_info.nrowminf;
		} else if (dbr_info.mod_type == DSC) {
			fromtid = ds_tid;
			got_tok = -1;
		}
		rtrn_code = holding_token();		
	    break;
	case TSTAMP:
		assert(!mf_row);
		replyback(got_tok, &my_pid, sizeof(pid_t));		
		dbr_info.t_stmp = dfs_msg->u.tst;
		msg_size = sizeof(tstamp_type) + sizeof(msg_hdr_type);
		sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
		DC_tstamp(&dfs_msg->u.tst);
		sigprocmask(SIG_UNBLOCK,&sigs,0);
		rtrn_code = holding_token();
 	    break;
	case DCDASCMD:
		assert(!mf_row);
		replyback(got_tok, &my_pid, sizeof(pid_t));				
 		msg_size = sizeof(dascmd_type) + sizeof(msg_hdr_type);
		switch (dfs_msg->u.dasc.type) {
		    case DCT_TM:
			switch (dfs_msg->u.dasc.val) {
			    case DCV_TM_START:
				dbr_info.tm_started = 1; break;
			    case DCV_TM_END:
				dbr_info.tm_started = 0; break;
			    default: break;
			}
			break;
		    case DCT_QUIT:
			switch (dfs_msg->u.dasc.val) {
			    case DCV_QUIT:
				switch (dbr_info.mod_type) {
				    /* ring clients don't bow out after QUIT */
				    case DRC: oper_loop = 0; break;
				    case DSC: DC_bow_out(); break;
				    default: msg(MSG_EXIT_ABNORM,"yikes who are you?");
				}
				break;
			    default: break;
			}
		    default: break;
		}
		sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
		DC_DASCmd(dfs_msg->u.dasc.type, dfs_msg->u.dasc.val);
		sigprocmask(SIG_UNBLOCK,&sigs,0);
		rtrn_code = holding_token();
		break;
    default: 
	    sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
	    rv = DC_other((unsigned char *)dfs_msg, got_tok);
		if (rv != DAS_NO_REPLY) replyback(got_tok, &rv, sizeof(reply_type));					    
	    break; 
    } /* switch */
    sigprocmask(SIG_UNBLOCK,&sigs,0);
return(rtrn_code);
}

/* disengage gracefully from dbr */
int DC_bow_out(void) {
  if (ds_tid<=0) oper_loop = 0;
  if (bow_out) return 0;
  msg(MSG,"task %d: bowing out",getpid());
  bow_out=1;
  return 0;
}
