/*
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/timers.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include <globmsg.h>
#include <cmdctrl.h>
#include <das.h>
#include <eillib.h>
#include <gw.h>
#include <dfs_mod.h>
#include "dggw_defs.h"
#include <rational.h>

/* defines */
#define HDR "dggw"
#define OPT_MINE ""

/* **************** */
/* global variables */
/* **************** */
char *minors;	    /* space for 3 minor frames */
pid_t tim_proxy;    /* timer proxy */
rational mfsec;	    /* minor frames per sec rational */
rational tr;	    /* for rowlets increment and decrement */
int limit;

/* command line option string */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_MINE;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */
int  i;
struct itimercb tcb;
struct itimerspec tval;
timer_t timer;
rational rat;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
    cc_init_options(argc,argv,0,0,0,0,NOTHING_ON_QUIT);

    /* initialisations */

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
		i=getopt(argc,argv,opt_string);
		switch (i) {
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default : break;
		}
    }  while (i!=-1);

    if ( (tim_proxy = qnx_proxy_attach(0, NULL, 0, -1)) == -1)
		msg(MSG_EXIT_ABNORM,"Can't attach proxy");
    tcb.itcb_event.evt_value = tim_proxy;
    if ( (timer = mktimer(TIMEOFDAY, _TNOTIFY_PROXY, &tcb)) ==-1)
	    msg(MSG_EXIT_ABNORM,"can't make timer");
    tval.it_value.tv_sec = TS_INTERVAL;
    tval.it_value.tv_nsec = 0L;
    tval.it_interval = tval.it_value;
    if (reltimer(timer, &tval, NULL) == -1)
		msg(MSG_EXIT_ABNORM, "Error in reltimer");    

    GW_init_options(argc,argv);
    DC_data_rows = dbr_info.nrowminf;

    /* allocate space */
    if ( !(minors = malloc(3 * tmi(nbminf))))
		msg(MSG_EXIT_ABNORM,"Can't allocate space for 3 minor frames");

    rat.num = tmi(nrowsper);
    rat.den = tmi(nsecsper);
    rdivideint(&rat, dbr_info.nrowminf, &mfsec);
    rtimesint(&mfsec, dbr_info.nrowminf * 10, &tr);
    rtimesint(&tr, tr.den, &rat);
    rdivideint(&rat, 10, &rat);
    rtimesint(&rat, SECDRIFT, &rat);
    limit = rat.num/rat.den;    
    
    /* main loop of a gw */
    GW_operate();
}
