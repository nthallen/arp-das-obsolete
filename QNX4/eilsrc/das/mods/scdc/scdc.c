/*
    Strobed Command Device Controller.
    Written by Nort.
    Modified by Eil July 1991 for QNX.
    Ported to QNX 4 by Eil 4/17/92.
*/

/* header files */
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/kernel.h>
#include <signal.h>
#include <sys/name.h>
#include <das.h>
#include <eillib.h>
#include <globmsg.h>
#include <scdc.h>
#include <dccc.h>

/* defines */
#define HDR "scdc"
#define OPT_MINE "w:"
#define WAIT_TIME 0

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE OPT_BREAK_INIT OPT_CC_INIT;


void main(int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */
char name[FILENAME_MAX+1];
unsigned int waittime;
char buf[MAX_MSG_SIZE];
reply_type replycode;
int msgsz, i;
pid_t from;
int dccc_tid;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    break_init_options(argc,argv);

    /* initialisations */
    waittime = WAIT_TIME;

    /* process args */
    opterr = 0;
    optind = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case 'w': waittime = atoi(optarg); break;
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default: break;
	}
    } while (i!=-1);

    /* register yourself */
    if ((qnx_name_attach(getnid(),LOCAL_SYMNAME(SCDC,name)))==-1)
	msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);

    /* look for DCCC */
    if ( (dccc_tid = qnx_name_locate(getnid(),LOCAL_SYMNAME(DCCC,name),0,0)) == -1)
	msg(MSG_EXIT_ABNORM,"Can't find %s",name);

    /* register with cmdctrl */
    cc_init_options(argc,argv,DCT_SCDC,DCT_SCDC,SC_MULTCMD,SC_MULTCMD,FORWARD_QUIT);

    /* program code */
    while (1) {

	/* receive msgs */
	buf[0] = DEATH;
	replycode = DAS_OK;
	while ( (from = Receive(0, buf, sizeof(buf) ))==-1)
	    msg(MSG_WARN, "error receiving message");

	msg(MSG_DEBUG,"received msg from task %d, header %d",from,buf[0]);

	/* check out msg structure */
	switch (buf[0]) {
	    case DASCMD:
		/* QUIT */
		if (buf[1] == DCT_QUIT && buf[2] == DCV_QUIT) {
		    Reply(from,&replycode,sizeof(reply_type));
		    DONE_MSG;
		}
		msgsz = sizeof(dascmd_type) + sizeof(msg_hdr_type);
		if (buf[1] == DCT_SCDC) buf[1] = DCT_DCCC;
		else {
		    replycode = DAS_UNKN;
		    msg(MSG_WARN,"unknown DASCMD type %d received",buf[1]);
		}
		break;
	    case SC_MULTCMD:
		msgsz = buf[1]+2;
		buf[0] = DC_MULTCMD;
		break;
	    default:
		replycode = DAS_UNKN;
		msg(MSG_WARN,"unknown msg header %d received",buf[0]);
		break;
	}

	if (Reply(from,&replycode,sizeof(reply_type))==-1)
	    msg(MSG_WARN,"error replying to task %d",from);

	if (replycode==DAS_OK) {
	    /* send to dccc */
	    while (Send(dccc_tid, buf, &replycode, msgsz, sizeof(reply_type))==-1)
		msg((errno==EINTR) ? MSG_WARN : MSG_EXIT_ABNORM,"Error sending to %s",DCCC);
	    if (replycode != DAS_OK) 
		msg(MSG_WARN,"Bad response code %d from %s",replycode,DCCC);

	    /* now wait */
	    if (waittime) delay(waittime);
		
	    /* send to dccc again */
	    if (Send(dccc_tid, buf, &replycode, msgsz, sizeof(reply_type))==-1)
		msg((errno==EINTR) ? MSG_WARN : MSG_EXIT_ABNORM,"Error sending to %s",DCCC);
	    if (replycode != DAS_OK) 
		msg(MSG_WARN,"Bad response code %d from %s",replycode,DCCC);
	} /* if */

    } /* while */

} /* main */
