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
#include <sys/types.h>
#include <das.h>
#include <eillib.h>
#include <dc.h>
#include <globmsg.h>
#ifdef __QNX__
#include <sys/psinfo.h>
#endif

#ifndef __QNX__
#define sigprocmask(x,y,z);
sigemptyset(x);
sigaddset(x,y);
sigaddset(x,y);
#endif

/* Globals */
token_type DC_data_rows;

/* Static variables */
static pid_t got_tok = 0;
static int ds_tid = 0;
static unsigned char bow_out = 0;
static unsigned char oper_loop;
static int my_pid;
static token_type mf_row;
static struct {
    hdr_type hdr;
    token_type n_rows;
} dr_tok = { DCTOKEN,0,0 };

/* the exit function during client operation ensuring proper bow-out */
void DC_exitfunction(void) {
pid_t pr;
if (oper_loop) {
    breakfunction(0);
	if (!bow_out) {
		switch (my_ipc) {
			default:
				/* pass on */
				holding_token();
				dr_tok.n_rows = 0;
				dr_tok.hdr.fromtid = getpid();
				/* kick a null token */
				pr = proxy_attach(ds_tid,(char *)&dr_tok,sizeof(dr_tok),-1,my_ipc);
				while (trig(pr,my_ipc,(char *)&dr_tok,sizeof(dr_tok))==-1)
					if (errno==EINTR) continue; else return;
				break;
			case IPC_PIPE:
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
hdr_type dr_init = { DCINIT,0 };
struct _mxfer_entry mlist[2];
#endif

    oper_loop=1;
    bow_out = 0;
    breakfunction(0);
    my_pid=getpid();
    my_ipc=IPC_ONLY(client_type);
    sigemptyset(&sigs);
    sigaddset(&sigs,SIGINT);
    sigaddset(&sigs,SIGTERM);
    alarmfunction(0);

    if (!IS_DC(client_type)) return 1;
    oper_loop=0;

    /* register data client exit function */
	if (atexit(DC_exitfunction))
		msg(MSG_EXIT_ABNORM,"Can't register DC exit function");

    /* ds_tid is source of data for a client, either dg or ds or stdin */
   	switch (DC_ONLY(client_type)) {
		case DSC:
			if ((ds_tid=name_locate(node, LOCAL_SYMNAME(DB_NAME), 0, 0, my_ipc)) == -1)
			    msg(MSG_EXIT_ABNORM,"Can't find symbolic name for %s on node %d",DB_NAME, node);
			setprio(getpid(),getprio(0)-1);
			/* this is for getting high pri proxies */		
			pflags(~0,_PPF_PRIORITY_REC,0,0,my_ipc);
			break;
		case DRC:
			if ((ds_tid=name_locate(0, GLOBAL_SYMNAME(DG_NAME), 0, 0, my_ipc)) == -1)
			    msg(MSG_EXIT_ABNORM,"Can't find symbolic name for %s",DG_NAME);
			pflags(~0,_PPF_SIGCATCH,0,0,my_ipc);
			break;
	}
			    
    if (ds_tid==my_pid) msg(MSG_EXIT_ABNORM,"My data source can't be myself");

	if (my_ipc == IPC_PIPE) {
		oper_loop=1;
		ds_tid=-1;
		dbr_info.mod_type = client_type;
		if (fd_read(STDIN_FILENO,(char *)&dbr_info,sizeof(dbr_info),0)==-1)
			msg(MSG_FATAL,"Can't read");
    } else {
	    /* get dbr_info */
	    dr_init.fromtid = my_pid;
	    setmx(&mlist[0], &dr_init, sizeof(hdr_type));
	    setmx(&mlist[1], &dbr_info, sizeof(dbr_info));
		while (!breaksignal)
			if ( !(oper_loop=!sndmx(ds_tid, 1, 2, &mlist, &mlist, my_ipc)) )
				if (errno != EINTR)
					msg(MSG_EXIT_ABNORM,"Error sending to my data source task %d",ds_tid);
				else continue;
			else break;
		if (!breaksignal)
		    if (dr_init.msg_type != DAS_OK) {
		    	oper_loop = 0;
				msg(MSG_EXIT_ABNORM,"bad response from data source task %d at registration",ds_tid);
			}
	}

	dbr_info.mod_type = client_type;    
    /* make space for data transfers */
    dfs_msg_size = dbr_info.max_rows * dbr_info.tm.nbrow + sizeof(dfs_msg_type);
	if ( !(dfs_msg = malloc(dfs_msg_size)))
		msg(MSG_EXIT_ABNORM,"Can't allocate %d bytes of memory",dfs_msg_size);
	dfs_msg->hdr.msg_type = DEATH;
    if (!DC_data_rows) DC_data_rows = dbr_info.max_rows;
	else if (DC_data_rows > dbr_info.max_rows) {
		msg(MSG_WARN,"ÿmin data msg size %d > %d allowable, defaulted",DC_data_rows,dbr_info.max_rows);
		DC_data_rows=dbr_info.max_rows;
	}
	
	if (dbr_info.mod_type==DSC) {
		/* make a request */
		got_tok = ds_tid;
		dfs_msg->hdr.msg_type = DCDATA;
		holding_token();
	}
	msg_size=1;

	if (breaksignal) msg(MSG_EXIT_NORM,"caught signal %d: exiting",breaksignal);
    return 0;
}

/* holding_token() handles all states while we have the token. */
static int holding_token(void) {
int rcv_buf;
int dr;
reply_type rv = REP_MAX + 1;
void *ptr;

if (got_tok < 1 || ds_tid < 1) return 0;
switch (dfs_msg->hdr.msg_type) {
	case DCDATA:
		if (DC_ONLY(dbr_info.mod_type)==DSC)
			if (got_tok==ds_tid || dfs_msg->u.drd.n_rows) return 0;
	case TSTAMP: case DCDASCMD: break;
	default: return 0; /* other message */
}
if (dbr_info.next_tid != ds_tid) {
	while (!breaksignal)
		if ( (got_tok=snd(dbr_info.next_tid,dfs_msg,&rcv_buf,msg_size, sizeof(pid_t),my_ipc))==-1)
			if (errno!=EINTR) break;
			else continue;
		else break;
} else {
	/* if null data for a ring client , dont pass on token */
	if (dbr_info.mod_type == DRC && dfs_msg->hdr.msg_type == DCDATA
		&& dfs_msg->u.drd.n_rows == 0 && !bow_out) return(0);
	if (dbr_info.tm_started && dbr_info.mod_type == DRC)
		/* data regulation */
		while (!breaksignal && !DC_data_rows) {
			if ((dr=rec(0, dfs_msg, dfs_msg_size,my_ipc,msg_size))==-1)
				if (errno!=EINTR) break;
				else continue;
			else switch (dfs_msg->hdr.msg_type) {
				case DCDATA: case TSTAMP: case DCDASCMD: 
				    msg(MSG_WARN,"Invalid msg received at data regulation stage: type %d",dfs_msg->hdr.msg_type);
					msg_size=1;					    
					break;
				default:
		            /* Eventually this may reply with a message unknown */
		            rv=DC_other((unsigned char *)dfs_msg, rcv_buf, &msg_size);
					if (rv!=DAS_NO_REPLY)
						while (rep(dr,&rv,sizeof(reply_type),my_ipc)==-1 && errno==EINTR);
		            break;
			}
		}
	if (dbr_info.mod_type == DSC) ptr = dfs_msg; else ptr = &rcv_buf;
	dr_tok.n_rows = DC_data_rows;
	dr_tok.hdr.fromtid = my_pid;
	while (!breaksignal)
		if ((got_tok=snd(dbr_info.next_tid,&dr_tok,ptr,sizeof(dr_tok),ptr==dfs_msg ? sizeof(dfs_msg_type) : sizeof(rcv_buf),my_ipc))==-1)
			if (errno!=EINTR) break;
			else continue;
		else break;
	if (rv!=REP_MAX+1) DC_data_rows=0;
}

switch (got_tok) {
	case -1:
		/* pass on of token failed */
		if (errno==ESRCH)  /* couldn't find neighbor */
			/* If it was the DG, quit. Otherwise send to DG from here on */
			if (dbr_info.next_tid == ds_tid) {
			    got_tok = ds_tid;
			    oper_loop = 0; /* no need to bow out */
			    return -1;
			} else {
			    if (dbr_info.next_tid)
				    msg(MSG,"my ring neighbor task %d bowed out",dbr_info.next_tid);
			    dbr_info.next_tid = ds_tid;
			    got_tok = ds_tid;
			}
			break;
		case 0:
			if (DC_ONLY(dbr_info.mod_type)==DSC) {
				msg_size=0;
				got_tok=ds_tid;
				break;
			}
			if (rcv_buf != dbr_info.next_tid && my_ipc != IPC_PIPE) {
				dbr_info.next_tid = rcv_buf;
				if (!rcv_buf) {
				    dbr_info.next_tid=ds_tid;
				    got_tok = dbr_info.next_tid;
				    holding_token();
				}
				msg(MSG,"my new neighbor task is %d",dbr_info.next_tid);
			}
			break;
	}
if (dbr_info.mod_type == DSC && DC_data_rows == 0) oper_loop = 0;
return 0;
}

/* client operation. returns 0 on success */
int DC_operate(void) {
int rtrn_code = 0;  /* Code returned to calling function */
int bower;

while (oper_loop && !breaksignal) {
	bower = 0;
	if (msg_size) {
		dfs_msg->hdr.msg_type = DEATH;
		/* receive for data */    	
		if ((got_tok=rec(0, dfs_msg, dfs_msg_size, my_ipc, msg_size)) < 1) {
			if (!got_tok) {
				msg(MSG,"reached end of file");
				DC_bow_out();
				bower=1;
			}
			else if (errno != EINTR) break; else continue;
		}
	}
	/* Handle bow_out conditions */
	if (bow_out && my_ipc != IPC_PIPE)
		switch (dbr_info.mod_type) {
			case DRC:
				if (!mf_row)
				    switch (dfs_msg->hdr.msg_type) {
						case DCDATA: case DCDASCMD: case TSTAMP:
							bower = 1;
						    if (dbr_info.next_tid == ds_tid) {
								my_pid=0;
								while (rep(got_tok, &my_pid, sizeof(pid_t),my_ipc)==-1 && errno==-1);
						    } else while (Relay(got_tok, dbr_info.next_tid)==-1 && errno==EINTR);
						default : break;
				    }
				    break;
			case DSC: if (DC_data_rows) DC_data_rows=0;
		}
    if (!bower) rtrn_code = DC_process_msg(got_tok);
	else  {
		oper_loop=0;
		ds_tid=0;
	}
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
switch (dfs_msg->hdr.msg_type) {
	case DEATH: msg_size = 1; break;
    case DCDATA:
		while (rep(got_tok, &my_pid, sizeof(pid_t),my_ipc)==-1 && errno==EINTR);
		if (dfs_msg->u.drd.n_rows) {
			msg_size = dfs_msg->u.drd.n_rows * tmi(nbrow) + sizeof(hdr_type) + sizeof(dbr_data_type) -1;
		    sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
		    DC_data(&dfs_msg->u.drd);
		    sigprocmask(SIG_UNBLOCK,&sigs,0);
		    mf_row = (mf_row + dfs_msg->u.drd.n_rows) % dbr_info.nrowminf;
		} else if (dbr_info.mod_type == DSC)
			/* data ready trigger */
			if (who != ds_tid) proxy_detach(who, my_ipc);
	    break;
	case TSTAMP:
		assert(!mf_row);
		while (rep(got_tok, &my_pid, sizeof(pid_t),my_ipc) && errno==EINTR);
		dbr_info.t_stmp = dfs_msg->u.tst;
		msg_size = sizeof(tstamp_type) + sizeof(hdr_type);
		sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
		DC_tstamp(&dfs_msg->u.tst);
		sigprocmask(SIG_UNBLOCK,&sigs,0);
 	    break;
	case DCDASCMD:
		assert(!mf_row);
		while (rep(got_tok, &my_pid, sizeof(pid_t),my_ipc)==-1 && errno==EINTR);
 		msg_size = sizeof(dascmd_type) + sizeof(hdr_type);
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
				    default: msg(MSG_WARN,"bad module type");
				    case DRC: oper_loop = 0; break;
				    case DSC: DC_bow_out(); break;
				}
				break;
			    default: break;
			}
		    default: break;
		}
		sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
		DC_DASCmd(dfs_msg->u.dasc.type, dfs_msg->u.dasc.val);
		sigprocmask(SIG_UNBLOCK,&sigs,0);
		break;
    default: 
	    sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
	    rv = DC_other((unsigned char *)dfs_msg, got_tok, &msg_size);
		if (rv != DAS_NO_REPLY)
			while (rep(got_tok, &rv, sizeof(reply_type),my_ipc) && errno==EINTR);
	    break; 
    } /* switch */
	rtrn_code = holding_token();				
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
