/*
    Memo Program.
    Written by Eil Aug 1992.
*/

/* header files */
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/psinfo.h>
#include <globmsg.h>
#include <memo.h>
#include <das_utils.h>

/* defines */
#define HDR "memo"
#define OPT_MINE "k:"

/* globals */
char *opt_string=OPT_MSG_INIT OPT_BREAK_INIT OPT_MINE;

main(int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

char recv_msg[MAX_MSG_SIZE];
char name[FILENAME_MAX+1];
struct {
    msg_hdr_type h;
    dascmd_type d;
} quit_msg = {DASCMD,DCT_QUIT,DCV_QUIT};
pid_t who;
nid_t n;
int i;
char rv = DAS_OK;

    /* initialise das options from command line */    
    msg_init_options(HDR,argc,argv);

    BEGIN_MSG;
    break_init_options(argc,argv);

    /* process args */
    opterr = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
 	    case 'k':
		n = atoi(optarg);
		if ( (who = qnx_name_locate(n,LOCAL_SYMNAME(MEMO,name),0,0))!=-1) {
		    if (Send(who,&quit_msg,&rv,sizeof(dascmd_type)+sizeof(msg_hdr_type),sizeof(reply_type))==-1)
			msg(MSG_EXIT_ABNORM,"error sending to %s: task %d",MEMO,who);
		    if (rv!=DAS_OK)
			msg(MSG_EXIT_ABNORM,"bad response from %s: task %d",MEMO,who);
		}
		else msg(MSG_EXIT_ABNORM,"Can't find %s on node %d",name,n);
		DONE_MSG;
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default: break;
	}
    } while (i!=-1);

    /* register yourself */
    if ((qnx_name_attach(getnid(),LOCAL_SYMNAME(MEMO,name)))==-1)
	msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);

    /* sort messages by priority */
    qnx_pflags(~0,_PPF_PRIORITY_REC,0,0);

    while(1) {
	rv = DAS_OK;
	errno=0;
	if ((who=Receive(0,recv_msg,MAX_MSG_SIZE))==-1)
	    msg(MSG_WARN,"error on receive");	
	else {
	    switch( recv_msg[0] ) {
		case MEMO_MSG:
		    if (Reply(who, &rv, sizeof(msg_hdr_type))==-1)
			msg(MSG_WARN,"error replying to task %d",who);
		    msg(MSG,"%s",recv_msg+sizeof(msg_hdr_type));
		    break;
		case DASCMD:
		    if ( recv_msg[1]==DCT_QUIT && recv_msg[2]==DCV_QUIT) {
			if (Reply(who, &rv, sizeof(msg_hdr_type))==-1)
			    msg(MSG_WARN,"error replying to task %d",who);
			msg_end();
			DONE_MSG;
			exit(0);
		    }
		default: rv = DAS_UNKN;
		    msg(MSG_WARN,"unrecognised msg received");
		    if (Reply(who, &rv, sizeof(msg_hdr_type))==-1)
			msg(MSG_WARN,"error replying UNKNOWN to task %d",who);
	    }
	}
    } /* while */
}
