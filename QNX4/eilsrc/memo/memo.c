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
#define OPT_MINE ""

/* globals */
char *opt_string=OPT_MSG_INIT OPT_BREAK_INIT OPT_MINE;

/* functions */
void my_signalfunction(int sig_number) {
    msg_end();
    exit(0);
}

main(int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

char recv_msg[MAX_MSG_SIZE];
char name[FILENAME_MAX+1];
pid_t tid_that_sent;
int i;
char rv = DAS_OK;

    /* initialise das options from command line */    
    msg_init_options(HDR,argc,argv);

    /* register yourself */
    if ((qnx_name_attach(getnid(),LOCAL_SYMNAME(MEMO,name)))==-1)
	msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);

    BEGIN_MSG;
    break_init_options(argc,argv);
    signal(SIGTERM,my_signalfunction);

    /* process args */
    opterr = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default: break;
	}
    } while (i!=-1);

    /* sort messages by priority */
    qnx_pflags(~0,_PPF_PRIORITY_REC,0,0);

    while(1) {
	errno=0;
	if ((tid_that_sent=Receive(0,recv_msg,MAX_MSG_SIZE))==-1)
	    msg(MSG_WARN,"error on receive");	
	else {
	    switch( recv_msg[0] ) {
		case MEMO_MSG:
		    if (Reply(tid_that_sent, &rv, sizeof(msg_hdr_type))==-1)
			msg(MSG_WARN,"error replying to task %d",tid_that_sent);
		    msg(MSG,"%s",recv_msg+sizeof(msg_hdr_type));
		    break;
		case DASCMD:
		    if ( recv_msg[1]==DCT_QUIT && recv_msg[2]==DCV_QUIT) {
			DONE_MSG;
			msg_end();
			exit(0);
		    }
		    break;
	    }
	}
    } /* while */
}
