/*
    Memo Program.
    Written by Eil Aug 1992.
*/

/* header files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/psinfo.h>
#include <memo.h>
#include <reply.h>
#include <eillib.h>

/* defines */
#define HDR "memo"
#define OPT_MINE "k:q"

/* globals */
char *opt_string=OPT_MSG_INIT OPT_BREAK_INIT OPT_MINE;

main(int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

char recv_msg[MEMO_MSG_MAX];
char name[NAME_MAX];
pid_t who;
nid_t n;
int i;
reply_type rv;
int audio_rec = 0;

    /* initialise das options from command line */    
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    break_init_options(argc,argv);

    /* process args */
    opterr = 0;
    optind = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case 'q': audio_rec = 1; break;
 	    case 'k':
		n = atoi(optarg);
		rv = MEMO_DEATH_HDR;
		if ( (who = qnx_name_locate(n,LOCAL_SYMNAME(MEMO,name),0,0))!=-1) {
		    if (Send(who,&rv,&rv,sizeof(reply_type),sizeof(reply_type))==-1)
			msg(MSG_EXIT_ABNORM,"error sending to %s: task %d",MEMO,who);
		    if (rv!=REP_OK)
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
	rv = REP_OK;
	errno=0;
	if ((who=Receive(0,recv_msg,MEMO_MSG_MAX))==-1)
	    msg(MSG_WARN,"error on receive");	
	else {
	    switch( recv_msg[0] ) {
		case MEMO_HDR:
		    if (Reply(who, &rv, sizeof(reply_type))==-1)
			msg(MSG_WARN,"error replying to task %d",who);
		    i=MSG;
		    if (audio_rec)
			if (strstr(recv_msg,FATAL_STR) || strstr(recv_msg,FAIL_STR))
			    i=MSG_FAIL;
			else if (strstr(recv_msg,WARN_STR)) i=MSG_WARN;
		    msg(i,"%s",recv_msg+1);
		    break;
		case MEMO_DEATH_HDR:
		    if (Reply(who, &rv, sizeof(reply_type))==-1)
			msg(MSG_WARN,"error replying to task %d",who);
		    msg_end();
		    DONE_MSG;
		    exit(0);
		default: rv = REP_UNKN;
		    msg(MSG_WARN,"unrecognised msg received");
		    if (Reply(who, &rv, sizeof(reply_type))==-1)
			msg(MSG_WARN,"error replying UNKNOWN to task %d",who);
	    }
	}
    } /* while */
}
