/*
    Nav Program: Reads navigation parameters on the ER2 and U2 aircraft
    from an asynchronous serial port at 9600 baud, 8 data bits, no parity,
    and 1 stop bit.
    Written by Eil 10/92.
*/

/* header files */
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <sys/timers.h>
#include <sys/kernel.h>
#include <sys/timers.h>
#include <sys/name.h>
#include <das.h>
#include <eillib.h>
#include <nortlib.h>
#include <globmsg.h>
#include <nav.h>

/* defines */
#define HDR "nav"
#define NAV HDR
/* a timeout flag, seconds and a default, valid when C flag given */
/* a quit flag, valid when C flag given, quit after time set or timeout */
#define OPT_MINE "CU:qQ"
#define FRAME_CHAR 0xAA
#define NAV_TIMEOUT 10
#define MAKEWORD(X) ( (buf[X] << 8) | buf[X+1] )

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE OPT_CC_INIT OPT_SERIAL_INIT;
short nav_params[5];
int terminated;
int timer_signal;
int set_clock;
int quitter;

/* functions */
void my_signalfunction(int sig_number) {
terminated = 1;    
}

void tim_signalfunction(int sig_number) {
timer_signal = 1;
if (set_clock > 0) set_clock = 0;
if (quitter) terminated = 1;
}

void main(int argc, char **argv) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */
int i,j,fr;
int msg_big_ctrl, msg_lit_ctrl;
unsigned char buf[28];
int fd;
timer_t timer_id;
struct timespec tp;
struct tm *timestruct;
struct itimerspec timer;
struct itimercb timercb;
int nav_hrs_from_mid;
long nav_secs_from_mid;
long sys_secs_from_mid;
long temp;
int nav_hrs, nav_mins, nav_secs;
int first_frame;
time_t timesecs;
short wd;
long timeout;
int name_attached;
int with_dg;
int quit_no_dg;
int jd;

    /* initialise msg options from command line */
    signal(SIGQUIT,my_signalfunction);
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;

    /* initialisations */
    first_frame = 1;
    terminated = 0;
    timer_signal = 0;
    set_clock = 0;
    msg_big_ctrl = 0;
    msg_lit_ctrl = 0;
    quitter = 0;
    timeout = -1;
    name_attached = 0;
    with_dg = 0;
    quit_no_dg = 0;
    jd = -1;

    /* process args */
    opterr = 0;
    optind = 0;
    do {
	i=getopt(argc,argv,opt_string);
	switch (i) {
	    case 'C':
		    if (seteuid(0) == -1)
			msg(MSG_WARN,"Can't set euid to root");
		    set_clock++;
		    break;
	    case 'q': quitter = 1; break;
	    case 'Q': quit_no_dg = 1; break;
	    case 'U': timeout = atol(optarg); break;
	    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
	    default: break;
	}
    } while (i!=-1);

    if (!set_clock) {
	if (quitter) {
	    msg(MSG_WARN,"option q not valid without option C: ignored");
	    quitter = 0;
	}
	if (timeout > 0) {
	    msg(MSG_WARN,"option T not valid without option C: ignored");
	    timeout = -1;
	}
    } else if (timeout < 0) timeout = NAV_TIMEOUT;

    if (optind >= argc) msg(MSG_EXIT_ABNORM,"no device specified");
    if (argc > optind+1) msg(MSG_EXIT_ABNORM,"only one device allowed");

    if ( (fd = open(argv[optind],O_RDONLY)) == -1)
	msg(MSG_EXIT_ABNORM,"Can't open %s",basename(argv[optind]));

    serial_init_options(argc, argv, fd);

    /* get a timer */
    if (timeout > 0) {
	timercb.itcb_event.evt_value = SIGUSR1;
	if ( (timer_id = mktimer(TIMEOFDAY, _TNOTIFY_SIGNAL, &timercb)) == -1)
	    msg(MSG_EXIT_ABNORM,"Can't attach timer");
	timer.it_value.tv_sec = timeout;
	timer.it_value.tv_nsec = 0L;
	timer.it_interval.tv_sec = 0L;
	timer.it_interval.tv_nsec = 0L;
    }

    /* register with cmdctrl */
    cc_init_options(argc,argv,0,0,0,0,QUIT_ON_QUIT);

    fr = 1;
    buf[0] = !(FRAME_CHAR);

    /* up timer */
    if (timeout > 0) {
	signal(SIGUSR1,tim_signalfunction);
	reltimer( timer_id,  &timer, NULL);
    }

    set_response(NLRSP_WARN); /* for norts Col_set_pointer */

    while (!terminated) {

	while (fr)
	    /* find the 2 framing characters */
	    if (dev_read(fd, &buf[1], 1, 1, 0, 0, 0, 0) == -1)
		/* signal QUIT or signal SIGUSR */
		if (terminated) break;
		/* signal SIGUSR */
		else if (timer_signal && !name_attached) {
		    if (qnx_name_attach(getnid(),LOCAL_SYMNAME(NAV))==-1)
			msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",NAV);
		    name_attached = 1;
		    continue;
		}
		/* error */
		else msg(MSG_EXIT_ABNORM,"error reading from %s",basename(argv[optind]));
	    else if (buf[0] == FRAME_CHAR && buf[1] == FRAME_CHAR) {
		msg(MSG,"found frame");
		break;
	    } else buf[0] = buf[1];

	i = fr ? 26 : 28;
	if (terminated) break;
	else if (timer_signal && !name_attached) {
	    if (qnx_name_attach(getnid(),LOCAL_SYMNAME(NAV))==-1)
		msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",NAV);
	    name_attached = 1;
	}
	
	/* then read rest of frame */
	while (dev_read(fd, fr ? &buf[2] : buf, i, i, 0, 0, 0, 0) == -1)
	    if (terminated) break;
	    else if (timer_signal && !name_attached) {
		if (qnx_name_attach(getnid(),LOCAL_SYMNAME(NAV))==-1)
		    msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",NAV);
		name_attached = 1;
		continue;
	    }
	    else msg(MSG_EXIT_ABNORM,"error reading from %s",basename(argv[optind]));

	if (terminated) break;

	/* valid first frame is available, disable timeout */
	if (timeout > 0) {
	    rmtimer(timer_id);
	    signal(SIGUSR1,SIG_DFL);
	    timeout = -1;
	}

	if (timer_signal && !name_attached) {
	    if (qnx_name_attach(getnid(),LOCAL_SYMNAME(NAV))==-1)
		msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",NAV);
	    name_attached = 1;
	}

	/* check frame out */
	if (!fr) {
	    /* check framing chars */
	    if (buf[0] != FRAME_CHAR || buf[1] != FRAME_CHAR) {
		fr = 1;
		buf[0] = !(FRAME_CHAR);
		msg(MSG_WARN,"bad frame");
		continue;
	    }
	}
	else fr = 0;

	/* get current system time */
	timesecs = time(NULL);
	timestruct = gmtime(&timesecs);

	/* settings on first frame */
	if (first_frame) {
	    if (buf[2] & 0x10) 
		msg(MSG,"Time Source: G.O.E.S. time code generator");
	    else msg(MSG,"Time Source: Unverified Internal Clock");
	    jd = (buf[2] & 0x03) * 100 + (buf[3] >> 4) * 10 + (buf[3] & 0x0F);
	    msg(MSG,"Julian Date: %d",jd);
	    first_frame = 0;
	}

	/* compare system time and nav time */
	nav_hrs_from_mid = (buf[2] & 0x20) ? 12 : 0;
	sys_secs_from_mid = (long)timestruct->tm_hour * 60 * 60 + (long)timestruct->tm_min * 60 + (long)timestruct->tm_sec;
	nav_hrs = nav_hrs_from_mid + (buf[4] * 4 + buf[5] / 60) / 60;
	nav_mins = (buf[4] * 4 + buf[5] / 60) % 60;
	nav_secs = buf[5] % 60;
	nav_secs_from_mid = (long)nav_hrs * 60 * 60 + (long)nav_mins * 60 + (long)nav_secs;

	if (terminated) break;

	/* set system clock on first frame */
	if (set_clock > 0) {
	    /* substitute in hours and seconds */
	    timestruct->tm_hour = nav_hrs;
	    timestruct->tm_min = nav_mins;
	    timestruct->tm_sec = nav_secs;
	    if (set_clock > 1) {
		/* figure day of month and months since Jan from Julian day */
		for (i=0;i<12;i++) {
		    j = jd;
		    switch (i) {
			case 1: /* feb */
			    if ( !((timestruct->tm_year+1900) % 4)) jd -= 29;
			    else jd -= 28;
			    break;
			case 3: case 5: case 8: case 10: jd -= 30; break;
			default: jd -= 31; break;
		    }
		    if (jd <= 0) break;
		}
		timestruct->tm_mon = i;
		timestruct->tm_mday = j;
	    }
	    /* set system clock */
	    tp.tv_sec = mktime(timestruct);
	    tp.tv_nsec = 0;
	    tzset();
	    tp.tv_sec -= timezone;
	    if ( (setclock(TIMEOFDAY,&tp)) == -1)
		msg(MSG_WARN,"Can't set clock");
	    else {
		msg(MSG,"setting system clock to %.24s %s",ctime(&tp.tv_sec),localtime(&tp.tv_sec)->tm_isdst ? tzname[1] : tzname[0]);
		/* get current system time */
		sys_secs_from_mid = nav_secs_from_mid;
	    }
	    set_clock = -1;
	    if (quitter) terminated = 1;
	    else  /* register yourself */
		if (qnx_name_attach(getnid(),LOCAL_SYMNAME(NAV))==-1)
		    msg(MSG_WARN,"Can't attach symbolic name for %s",NAV);
	}

	temp = nav_secs_from_mid - sys_secs_from_mid;
	if (temp > SHRT_MAX) {
	    if (!msg_big_ctrl) {
		msg(MSG,"time deviation too big by %ld", temp - (temp % SHRT_MAX));
		msg_big_ctrl = 1;
		if (msg_lit_ctrl) msg_lit_ctrl = 0;
	    }
	    temp %= SHRT_MAX;
	}
	if (temp < SHRT_MIN) {
	    if (!msg_lit_ctrl) {
		msg(MSG,"time deviation too small by %ld", labs(temp) - labs(temp % SHRT_MIN));
		msg_lit_ctrl = 1;
		if (msg_big_ctrl) msg_big_ctrl = 0;
	    }
	    temp %= SHRT_MIN;
	}

	if (!with_dg)
	    if (!(Col_set_pointer(NAV_FLAG_ID, nav_params, 0))) {
		with_dg = 1;
		msg(MSG,"achieved cooperation with DG");
	    } else
		if (quit_no_dg)
		    msg(MSG_EXIT_ABNORM,"Can't cooperate with DG");
		else {
		    errno = 0;
		    set_response(NLRSP_QUIET); /* for norts Col_set_pointer */
		}

	wd = temp;
	/* form words and update array */
	msg(MSG_DEBUG,"time deviation: %d",wd);
	nav_params[0] = wd;
	wd = MAKEWORD(6); /* lattitude */
	msg(MSG_DEBUG,"lattitude: %d",wd);
	nav_params[1] = wd;
	wd = MAKEWORD(8); /* longitude */
	msg(MSG_DEBUG,"longitude: %d",wd);
	nav_params[2] = wd;
	wd = MAKEWORD(20); /* altitude */
	msg(MSG_DEBUG,"altitude: %d",wd);
	nav_params[4] = wd;
	wd = MAKEWORD(22); /* true air speed */
	msg(MSG_DEBUG,"true air speed: %u",wd);
	nav_params[3] = wd;

    } /* while */

    DONE_MSG;

} /* main */
