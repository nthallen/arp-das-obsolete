/*
    DBR Data Generator file descriptor in.
    Written June 1993 by Eil for QNX 4.
*/

/* includes */
#include <stdlib.h>
#include <sys/timers.h>
#include <globmsg.h>
#include <das.h>
#include <dbr.h>
#include <dbr_mod.h>
#include <nortlib.h>
#include <eillib.h>
#include <rational.h>
#include <fdin.h>

/* defines */
#define HDR "fin"
#define OPT_MINE

/* global variables */
char *opt_string=OPT_DG_INIT OPT_DG_DAC_IN OPT_MSG_INIT OPT_CC_INIT OPT_MINE;
char *minors;	    /* space for 3 minor frames */
int fd;			    /* descriptor to read from */
pid_t tim_proxy;    /* timer proxy */
rational mfsec;	    /* minor frames per sec rational */
rational tr;	    /* for rowlets increment and decrement */
int limit;
int quitter = 1;

main( int argc, char **argv )  {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */		
int i;
struct itimercb tcb;
struct itimerspec tval;
timer_t timer;
rational rat;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;
	cc_init_options(argc,argv, DCT_TM, DCT_TM, 0, 0, FORWARD_QUIT);

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
		i=getopt(argc,argv,opt_string);
		switch (i) {
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default : break;
		}
    }  while (i!= -1);

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

    /* read relevant information */
    if (DG_dac_in(argc,argv) !=0)
		msg(MSG_EXIT_ABNORM,"Can't dac initialise");

    /* allocate space */
    if ( !(minors = malloc(3 * tmi(nbminf))))
		msg(MSG_EXIT_ABNORM,"Can't allocate space for 3 minor frames");

    rat.num = tmi(nrowsper);
    rat.den = tmi(nsecsper);

    /* initialise data generator */
    if (DG_init_options(argc,argv) != 0)
		msg(MSG_EXIT_ABNORM,"Can't DG initialise");

    rdivideint(&rat, dbr_info.nrowminf, &mfsec);
    rtimesint(&mfsec, dbr_info.nrowminf * 10, &tr);
    rtimesint(&tr, tr.den, &rat);
    rdivideint(&rat, 10, &rat);
    rtimesint(&rat, SECDRIFT, &rat);
    limit = rat.num/rat.den;

    DG_operate();
}
	
