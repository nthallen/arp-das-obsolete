/*
	Command Control Program.
	Written by David Stahl, 1991.
	Modified Aug 21 1991 by Eil, to change result_type
	Modified and Ported to QNX 4 by Eil 4/20/92.
*/
/*	Relays are used which don't work over the network, thus processes
	that send to cmdctrl and process that recieve from cmdctrl by relay
	are located on the same node.
*/

/* header files */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <process.h>
#include <sys/psinfo.h>
#include <sys/kernel.h>
#include <sys/types.h>
#include <sys/name.h>
#include <sys/sys_msg.h>
#include <sys/sched.h>
#include <globmsg.h>
#include <cmdctrl.h>
#include <das_utils.h>
#include <memo.h>

/* defines */
#define HDR	    "ctrl"
#define OPT_MINE    "r:d:Rp:PMn:mt"
#define MAX_MSG_TYP  MAX_GLOBMSG    /* Maximum number of different message types */
#define MAX_IDS      30		    /* Maximum number of active valid ids at a time */
#define STARTER_PROG "spwnr"
#define MSG_TYPE_NOT_FOUND   0
#define HASH_SIZE     MAX_IDS	    /* size of id hash table */
/* Disabling the death messages is necessay when rebooting from cmdctrl */
#define REBOOTSYSTEM \
    { \
	qnx_pflags(0,_PPF_INFORM,0,0); \
	msg(MSG,"preceeding events call for a reboot"); \
	system("shutdown -f please"); \
    }

/* Structures and Enumerated types */
typedef int relay_id_type;
typedef struct {
    quit_type how_to_quit;	/* Action to take on DASCmd quit */
    death_type how_to_die;	/* Action on death of a task */
    char *task_start;
} handle_quit_type;
typedef struct {
    pid_t id;
    char task_name[_POSIX_NAME_MAX];
    handle_quit_type *ptr;
} hash_type;
    
/* global variables */
char *opt_string = OPT_MSG_INIT OPT_BREAK_INIT OPT_MINE;
char command[80];	/* holds DAS startup command */
char name[FILENAME_MAX+1];
char fullname[MAX_MSG_SIZE];
char starter_prog[FILENAME_MAX];	    
pid_t strt_tid;
pid_t memo_tid;
relay_id_type relay_msgs[MAX_MSG_TYP]={0};      /* array of how to relay message types */ 
relay_id_type relay_dascmd_arr[MAX_MSG_TYP]={0};/* array of how to relay DASCMDs */
hash_type table[MAX_IDS]={0};			/* hash table of ids */
int cmdctrl_active;				/* Boolean to consider whether or not to quit main loop */
int cmd_id;					/* id to attach my name */
int num_deaths;					/* num_deaths since my birth */
int die_num;					/* max deaths to tolerate before rebooting */
int reboot_when_bad;				/* flags */
int reboot_on_strt_death;
int reboot_on_memo_death;
int kill_memo;
int need_strt;
char memo_prog[FILENAME_MAX];

/* function declarations */
void my_exitfunction(void) {
    shut_down();
    if (strt_tid != -1) {
	msg(MSG,"terminating %s",starter_prog);
	kill (strt_tid, SIGQUIT);
    }
}

/* this is for catching SIGPWR */
void my_sigpwrfunction(int sig_number) {
/* don't do an exit, don't shut down tasks */
    flushall();
    fcloseall();
}

/* this will catch all bad terminating signals and REBOOT */
void my_signalfunction(int sig_number) {
    msg(MSG_FAIL,"Caught BAD Signal %d",sig_number);
    if (reboot_when_bad) REBOOTSYSTEM;
    exit(1);
}

void initialize (void);
void handle_ccreg (pid_t id_to_relay_to, ccreg_type *ccreg_msg, int hsh);
void handle_death (pid_t id, int hsh);
void relay_msg (pid_t src_tid, msg_hdr_type message_type);
void reply_msg (pid_t reply_to_tid, reply_type message_byte);
void relay_dascmd (pid_t src_tid, dasc_msg_type * recv_msg);
relay_id_type find_dest (int msg_type);
relay_id_type find_dasc (int dasc_type);
void shut_down (void);
void cmd_ctrl_loop (void);
void do_relay (pid_t src_tid, pid_t dst_tid);
void release_relays (int down_id, int hsh);


main (int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */
int i;

    signal(SIGABRT,my_signalfunction);
    signal(SIGFPE,my_signalfunction);
    signal(SIGHUP,my_signalfunction);
    signal(SIGILL,my_signalfunction);
    signal(SIGSEGV,my_signalfunction);
    signal(SIGUSR1,my_signalfunction);
    signal(SIGUSR2,my_signalfunction);
    signal(SIGBUS,my_signalfunction);
    signal(SIGDEV,my_signalfunction);

    /* initialise das options from command line */    
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    break_init_options(argc,argv);
    BREAK_SETUP;
    /* replace the signal function given in break.c */
    signal(SIGPWR,my_sigpwrfunction);

    /* set exit function */
    if (atexit(my_exitfunction))
	msg(MSG_WARN,"Can't register exit function");

    /* initialisations */
    if (seteuid(0)==-1) msg(MSG_EXIT_ABNORM,"Can't set euid to root");
    qnx_pflags(~0L,_PPF_INFORM | _PPF_PRIORITY_REC,0,0);
    cmdctrl_active = 1;
    need_strt=0;
    die_num=INT_MAX;
    strt_tid = -1;
    memo_tid = -1;
    reboot_when_bad=0;
    reboot_on_strt_death=0;
    strcpy(starter_prog,STARTER_PROG);
    strncpy(memo_prog,MEMO,FILENAME_MAX-1);
    num_deaths = 0;
    command[0] = '\0';

    /* process args */
    opterr = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case 'd': strncpy(command,optarg,79);  break;
	    case 'r': die_num = atoi(optarg);  break;
	    case 'R': reboot_when_bad=1;  break;
	    case 'P': reboot_on_strt_death=1;  break;
	    case 'M': reboot_on_memo_death=1;  break;
	    case 't': need_strt=1; break;
	    case 'p': strncpy(starter_prog,optarg,FILENAME_MAX-1); break;
	    case 'n': strncpy(memo_prog,optarg,FILENAME_MAX-1); break;
	    case 'm': kill_memo=1; break;
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default : break;
	}
    } while (i!=-1);

    if ( (cmd_id=qnx_name_attach(0, LOCAL_SYMNAME(CMD_CTRL, name))) ==-1)
	msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);

    /* look for memo */
    if (reboot_on_memo_death || kill_memo)
	if ( (memo_tid = qnx_name_locate(getnid(), LOCAL_SYMNAME(MEMO,name),0,0))==-1)
	    msg(MSG_WARN,"can't find %s",basename(memo_prog));

    /* program code */
    initialize();
    cmd_ctrl_loop();
    exit(0); /* and shut_down is called */
}

void initialize(void) {
int loop_var;

    for (loop_var=0; loop_var < MAX_MSG_TYP; loop_var++) {
	relay_msgs[loop_var]=0;
	relay_dascmd_arr[loop_var]=0;
    }
    for (loop_var=0; loop_var < MAX_IDS; loop_var++) {
	table[loop_var].id=0;
	if (table[loop_var].ptr) {
	    if (table[loop_var].ptr->task_start)
		free(table[loop_var].ptr->task_start);
	    free(table[loop_var].ptr);
	    table[loop_var].ptr = 0;
	}
    }
}

void handle_ccreg (pid_t new_tid, ccreg_type *ccreg_msg, int hsh) {
/* Check to see if message types do not overlap with existing types
   * and then check to see if quit is ok, If all checks out then add the
   * item to the structure in question.
*/
    register int check_it;
    int no_msgs;
    int no_dascmds;
    struct _psinfo ps;
    unsigned char ccreg_state;

    ccreg_state = DAS_OK;
    if (hsh>=HASH_SIZE) ccreg_state=DAS_BUSY;
    else {
	if  ( !((ccreg_msg->min_dasc == 0) && (ccreg_msg->max_dasc == 0))) {
	    no_dascmds = 0;
	    for (check_it = ccreg_msg->min_dasc; check_it <= ccreg_msg->max_dasc && check_it < MAX_MSG_TYP; check_it++)
		if (relay_dascmd_arr[check_it] != 0) {
		    ccreg_state = DAS_BUSY;
		    msg(MSG_WARN,"registration with DASCMD type %d conflicts with task %d",check_it,relay_dascmd_arr[check_it]);
		    break;
		}
        }
	else no_dascmds = 1;
	if  ( (!((ccreg_msg->min_msg == 0) && (ccreg_msg->max_msg == 0))) && (ccreg_state == DAS_OK)) {
	    no_msgs = 0;
	    for (check_it = ccreg_msg->min_msg; check_it <= ccreg_msg->max_msg && check_it < MAX_MSG_TYP; check_it++)
		if (relay_msgs[check_it] != 0) {
		    ccreg_state = DAS_BUSY;
		    msg(MSG_WARN,"registration with msg type %d conflicts with task %d",check_it,relay_msgs[check_it]);
		    break;
		}
        }  else no_msgs = 1;
	if (table[hsh].ptr) {
	    ccreg_state = DAS_BUSY;
	    msg(MSG_WARN,"task %d: %s: already registered",new_tid,table[hsh].task_name);
	}
	/* Check quit state */
	if (ccreg_msg->how_to_quit > MAX_QUIT_TYPE || ccreg_msg->how_to_die > MAX_DEATH_TYPE) {
	    ccreg_state = DAS_UNKN;
	    msg(MSG_WARN,"unknown death or quit type");
	}

	if (ccreg_state == DAS_OK) {
	    if ((ccreg_msg->how_to_die==TASK_RESTART || ccreg_msg->how_to_die==DAS_RESTART)
		&& strt_tid==-1 && (strt_tid=spawnlp(P_NOWAIT,starter_prog,starter_prog,NULL)) ==-1) {
		msg(need_strt ? MSG_EXIT_ABNORM : MSG_WARN,"Can't spawn %s",starter_prog);
		ccreg_state = DAS_BUSY;
	    }
	    else {
		table[hsh].id=new_tid;
		if ( (qnx_psinfo(PROC_PID,new_tid,&ps,0,0)) !=new_tid) {
		    msg(MSG_WARN,"can't get name of task %d",new_tid);
		    sprintf(table[hsh].task_name,"task %d",new_tid);
		}
		else strncpy(table[hsh].task_name,basename(ps.un.proc.name),_POSIX_NAME_MAX-1);
		msg(MSG,"registering %s: task %d",table[hsh].task_name,new_tid);
		table[hsh].ptr=(handle_quit_type *)malloc(sizeof(handle_quit_type));
		table[hsh].ptr->how_to_quit = ccreg_msg->how_to_quit;
		table[hsh].ptr->how_to_die = ccreg_msg->how_to_die;
		if (ccreg_msg->how_to_die==TASK_RESTART) {
		    table[hsh].ptr->task_start=(char *)malloc(strlen(ccreg_msg->task_start)+1);
		    strcpy(table[hsh].ptr->task_start,ccreg_msg->task_start);
		}
		if (no_dascmds != 1)
		    for (check_it = ccreg_msg->min_dasc; check_it <= ccreg_msg->max_dasc && check_it < MAX_MSG_TYP; check_it++)
			relay_dascmd_arr[check_it] = new_tid;
		if (no_msgs != 1)
		    for (check_it = ccreg_msg->min_msg; check_it <= ccreg_msg->max_msg && check_it < MAX_MSG_TYP; check_it++)
			relay_msgs[check_it] = new_tid;
	    }
	}
    }
    reply_msg (new_tid, ccreg_state);
    return;                
}

void relay_msg (pid_t src_tid, msg_hdr_type message_type) {
    pid_t dst_tid;    /* Tid where message should be relayed to */
    if ( (dst_tid = find_dest (message_type)) == MSG_TYPE_NOT_FOUND)
        reply_msg (src_tid, DAS_UNKN);
    else
        do_relay (src_tid, dst_tid);
    return;
}

void reply_msg (pid_t reply_to_tid, reply_type message_byte) {
    reply_type result_msg;
    char m[8]="UNKNOWN";
    result_msg = message_byte;
    if (message_byte == DAS_BUSY) strcpy(m,"BUSY");
    if (message_byte == DAS_BUSY || message_byte == DAS_UNKN)
	msg(MSG_WARN,"replied %s to task %d",m,reply_to_tid);
    Reply (reply_to_tid, &result_msg, sizeof(reply_type));
    return;
}

void relay_dascmd (pid_t src_tid, dasc_msg_type * recv_msg) {
    pid_t dst_tid;    /* Tid where message should be relayed to */
    msg(MSG,"received DASCMD: type %d, value %d, from task %d",recv_msg->dascmd.type,recv_msg->dascmd.val,src_tid);
    if ( (recv_msg->dascmd.type == DCV_QUIT) && (recv_msg->dascmd.val == DCV_QUIT)) {
	reply_msg (src_tid, DAS_OK);
	cmdctrl_active = 0;
    }
    else if ( (dst_tid = find_dasc (recv_msg->dascmd.type)) == MSG_TYPE_NOT_FOUND)
        reply_msg (src_tid, DAS_UNKN);
    else do_relay (src_tid, dst_tid);
    return;
}

relay_id_type find_dest (int msg_type) {
    relay_id_type return_id;
    if (msg_type >= MAX_MSG_TYP) return(MSG_TYPE_NOT_FOUND);
    if ( (return_id = relay_msgs[msg_type]) == 0)
        return_id = MSG_TYPE_NOT_FOUND;
    return return_id;
}

relay_id_type find_dasc (int dasc_type) {
    relay_id_type return_id;
    if (dasc_type >= MAX_MSG_TYP) return(MSG_TYPE_NOT_FOUND);
    if ( (return_id = relay_dascmd_arr[dasc_type]) == 0)
        return_id = MSG_TYPE_NOT_FOUND;
    return return_id;
}

/* shut down cmdctrl-registered tasks and MEMO */
void shut_down (void) {
    dasc_msg_type kill_message = {DASCMD, DCV_QUIT, DCV_QUIT};
    char recv_msg;
    int loop_var;

    qnx_pflags(0,_PPF_INFORM,0,0);

    msg(MSG,"shutting down");
    for (loop_var = 0; loop_var < HASH_SIZE; loop_var++)
	if (table[loop_var].ptr)
	    switch (table[loop_var].ptr->how_to_quit) {
		case NOTHING_ON_QUIT: break;
		case FORWARD_QUIT:
		    Send (table[loop_var].id, &kill_message, &recv_msg, sizeof(dasc_msg_type), sizeof(reply_type));
		    break;
		case SET_BREAK:
		    kill (table[loop_var].id, SIGINT);
		    break;
		case TERM_ON_QUIT:
		    kill (table[loop_var].id, SIGTERM);
		    break;
		case QUIT_ON_QUIT:
		    kill (table[loop_var].id, SIGQUIT);
		    break;
	    }

    /* do this with lower priority */
    loop_var = getprio(0);
    if (loop_var > 2) setprio(getpid(),loop_var-2);
    msg(MSG,MSG_DONE);
    if (kill_memo) {
	if (memo_tid==-1)
	    memo_tid = qnx_name_locate(getnid(), LOCAL_SYMNAME(MEMO,name),0,0);
	if (memo_tid !=-1)
	    Send (memo_tid, &kill_message, &recv_msg, sizeof(dasc_msg_type), sizeof(reply_type));
    }

    return;
}

int get_hash(pid_t tid_that_sent) {
int i;
int hsh;
for (i=0,hsh=tid_that_sent%MAX_IDS;
     i<HASH_SIZE && table[hsh].id && table[hsh].id!=tid_that_sent;
	hsh=(hsh+1)%MAX_IDS) i++;
if (i>=HASH_SIZE) {
    msg(MSG_WARN,"Too many clients");
    return(HASH_SIZE);
}
return(hsh);
}

void cmd_ctrl_loop (void) {
    pid_t   tid_that_sent;
    unsigned char  recv_msg[MAX_MSG_SIZE];
    struct _sysmsg_hdr_reply death_reply={EOK,0};

    while (cmdctrl_active == 1) {

	if ((tid_that_sent=Receive(0,recv_msg,MAX_MSG_SIZE))==-1)
	    msg(MSG_WARN,"error on receive");
	else {
	    BREAK_PROTECT;
	    switch (recv_msg[0])  {
		case DEATH:
		    Reply(tid_that_sent,&death_reply,sizeof(struct _sysmsg_hdr_reply));
		    handle_death(tid_that_sent, get_hash(tid_that_sent));
		    break;
		case DASCMD:
		    relay_dascmd (tid_that_sent, (dasc_msg_type *) recv_msg);
		    break;
		case CCReg_MSG:
		    handle_ccreg (tid_that_sent, (ccreg_type *) recv_msg, get_hash(tid_that_sent));
		    break;
		default:
		    /* Call routine to find where to relay message */
		    relay_msg (tid_that_sent,recv_msg[0]);
		    break;
	    }
	BREAK_ALLOW;
	}
    }
}

void do_relay (int src_tid, int dst_tid) {
/*    msg(MSG,"relayed from %d to %d",src_tid,dst_tid);*/
    Relay (src_tid, dst_tid);
    return;
}

void handle_death (pid_t tid, int hsh) {
reply_type replycode=DAS_UNKN;

    if (tid==strt_tid) {
	msg(MSG,"%s died: task %d",starter_prog,strt_tid);
	if (reboot_on_strt_death) REBOOTSYSTEM;
	strt_tid=-1;
    }
    if (tid==memo_tid) {
	msg(MSG,"%s died: task %d",memo_prog,memo_tid);
	if (reboot_on_memo_death) REBOOTSYSTEM;
	memo_tid=-1;
    }

    if (hsh>=HASH_SIZE) return;
    if (!table[hsh].id) return;

    msg(MSG,"%s died: task %d", table[hsh].task_name, tid);
    if (++num_deaths==die_num ) {
	msg(MSG,"there have been %d deaths",die_num);
	REBOOTSYSTEM;
    }

    if (table[hsh].ptr)
	    switch (table[hsh].ptr->how_to_die) {
		case NOTHING_ON_DEATH:
			release_relays(tid, hsh);
			break;
		case TASK_RESTART:
			msg(MSG,"restarting: %s",basename(table[hsh].ptr->task_start));
			sprintf(fullname,"%c%s",TASK_RESTART,table[hsh].ptr->task_start);
			if (strt_tid!=-1)
			    if ( (Send(strt_tid,fullname,&replycode,strlen(fullname)+1,sizeof(reply_type)))==-1)
				msg(MSG_WARN,"Can't send to %s",starter_prog);
			    else if (replycode !=DAS_OK)
				msg(MSG_WARN,"Bad response %d from %s",replycode,starter_prog);
			else msg(MSG_WARN,"%s dosn't exist: can't restart: %s",starter_prog,basename(table[hsh].ptr->task_start));
			release_relays(tid, hsh);
			break;
		case DAS_RESTART:
			/* undo my attached name */
			qnx_name_detach(0, cmd_id);
			release_relays(tid, hsh);
			/* shut down all registered tasks and maybe MEMO */
			shut_down();
			msg(MSG,"restarting: %s",command);
			sprintf(fullname,"%c%s",DAS_RESTART,command);
			if (strt_tid!=-1) {
			    if ( (Send(strt_tid,fullname,&replycode,strlen(fullname)+1,sizeof(reply_type)))==-1)
				msg(MSG_WARN,"Can't send to %s",starter_prog);
			    else if (replycode !=DAS_OK)
				msg(MSG_WARN,"Bad response %d from %s",replycode, starter_prog);
			    kill (strt_tid, SIGQUIT); /* in case bad response */
			    _exit(0);
			} else msg(MSG_WARN,"%s dosn't exist: can't restart: %s",starter_prog,command);
			_exit(1); /* don't call exitfunction */
			break;
		case SHUTDOWN:
			release_relays(tid, hsh);
			cmdctrl_active = 0;
			break;
		case REBOOT:
			REBOOTSYSTEM;
			break;
		default: msg(MSG_WARN,"invalid death code %d",table[hsh].ptr->how_to_die);
	    }
}

void release_relays (pid_t down_id, int hsh) {
    int loop_var;       
    for (loop_var = 0; loop_var < MAX_MSG_TYP; loop_var++) {
        if (relay_msgs [loop_var] == down_id)
            relay_msgs [loop_var] = 0;
        if (relay_dascmd_arr [loop_var] == down_id)
            relay_dascmd_arr [loop_var] = 0;
    }
    if (table[hsh].ptr) {
	if (table[hsh].ptr->how_to_die==TASK_RESTART)
	    free(table[hsh].ptr->task_start);
	free(table[hsh].ptr);
	table[hsh].ptr=0;
    }
    return;
}
