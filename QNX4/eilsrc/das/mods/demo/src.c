/*
    This is a demo source file for programs int the data aquisition system,
    but not ino the data buffered ring.
*/

/* header files */
#include <stdio.h>
#include <stdlib.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <das_utils.h>
/* #include <cmdctrl.h> */
/* #include <globmsg.h> */

/* function declarations */
void exitfunction(void) {
    qnx_name_detach(getnid(),<>_id);
    msg_end();
    flushall();
    fcloseall();
}

void breakfunction(int sig_number) {
    BREAK_MSG;
}

/* defines */
#define HDR <hdr>

/* global variables */


main (int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */
char name[FILENAME_MAX+1];

    /* usage message */
    if (need_usage( argv )) usage(EXIT_SUCCESS, argv);
    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    /* set exit function */
    if (atexit(exitfunction))
	msg(MSG_WARN,"Can't register exit function");
    /* set break function */
    if (signal(SIGINT,breakfunction)==SIG_ERR)
	msg(MSG_WARN,"Can't register break function");

    /* initialisations */

    /* process remaining arguments */
    opterr = 0;
    while ((c=getopt(argc,argv,".")) !=-1)
	switch (c) {
	    case '.': break; /* msg option */
	    case '?': msg(MSG_WARN, "No such option: -%c",optopt);
			usage(EXIT_FAILURE, argv); break;
	}

    /* process additional command line args (if need) */
    if (optind >= argc) {
	msg(MSG_WARN,"<msg>");
	usage(EXIT_FAILURE, argv); break;
    }

    /* register yourself ( if need ) */
    if (( <>_id=qnx_name_attach(getnid(),LOCAL_SYMNAME(<>,name)))==-1)
	msg(MSG_EXIT_ABNORM,"Can't attach name %s",name);

    /* look for companions ( if need ) */
    if ( (<>_tid = qnx_name_locate(getnid(),LOCAL_SYMNAME(<>,name),0)) == -1)
	msg(MSG_EXIT_ABNORM,"Can't find %s",name);

    /* look for companions ( cmdctrl ) */
    if ( (cmd_tid = qnx_name_locate(getnid(),LOCAL_SYMNAME(CMD_CTRL,name),0)) == -1)
	msg(MSG_EXIT_ABNORM,"Can't find %s",name);

    /* register with cmdctrl ( if need ) */
    reg.ccreg_byt  = CCReg_MSG;
    reg.min_dasc = reg.max_dasc = DCT_SCDC;
    reg.min_msg = reg.max_msg = MULTCMD;
    reg.how_to_quit = FORWARD_QUIT;
    reg.how_to_quit = FORWARD_QUIT;
    if (!(Send( cmd_tid, &reg, &replycode, sizeof(reg), 1 )))
	msg(MSG_EXIT_ABNORM,"Error sending to %s",CMD_CTRL);

    if (replycode != DAS_OK)
	msg(MSG_EXIT_ABNORM, "Bad response from %s",CMD_CTRL);

    /* program code */


    DONE_MSG;

}
