/*
    Pif Program: Reads navigation parameters on the Perseus aircraft
    from an asynchronous serial port at 9600 baud, 8 data bits, no parity,
    and 1 stop bit.
    Written by Eil 7/93.
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
#include "das.h"
#include "eillib.h"
#include "nortlib.h"
#include "globmsg.h"
#include "pif.h"

/* defines */
#define HDR "pif"
#define PIF HDR
#define FRAME_SIZE 36
#define OPT_MINE "CU:qQ"
#define FRAME_CHAR 0x05
#define PIF_TIMEOUT 10
#define MAKEWORD(X) ( (buf[X] << 8) | buf[X+1] )
#define ATTACH_NAME \
{ \
if (qnx_name_attach(getnid(),LOCAL_SYMNAME(PIF))==-1) \
  msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",PIF); \
name_attached = 1; \
}

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE OPT_CC_INIT OPT_SERIAL_INIT;
short pif_params[5];
int terminated;
int timer_signal;
int set_clock;
int quitter;
static struct {
  unsigned char sync;
  PIF_FRAME fr;
  unsigned short crc;
} buf;

static PIF_FRAME outbuf;

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
  int fd;
  timer_t timer_id;
  struct timespec tp;
  struct tm *timestruct;
  struct itimerspec timer;
  struct itimercb timercb;
  int pif_hrs_from_mid;
  long pif_secs_from_mid;
  long sys_secs_from_mid;
  long temp;
  int pif_hrs, pif_mins, pif_secs;
  int first_frame;
  time_t timesecs;
  short wd;
  long timeout;
  int name_attached;
  int with_dg;
  int quit_no_dg;
  int jd, last_jd, dom, last_dom, jd_i, dom_i;
  long int c;
  unsigned int good_frames, bad_frames;

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
  bad_frames = 0;
  good_frames = 0;
  jd = INT_MAX;
  dom = -1;
  jd_i = -1;
  dom_i = -1;

  /* process args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case 'C':
      if (seteuid(0) == -1)
	msg(MSG_WARN,"Can't set euid to root");
      set_clock=1;
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
  } else if (timeout < 0) timeout = PIF_TIMEOUT;

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
  buf.sync = !(FRAME_CHAR);

  /* up timer */
  if (timeout > 0) {
    signal(SIGUSR1,tim_signalfunction);
    reltimer( timer_id,  &timer, NULL);
  }

  set_response(NLRSP_WARN);	/* for norts Col_set_pointer */

  while (!terminated) {

    while (fr)
      /* find 1 synch character */
      if (dev_read(fd, &buf.sync, 1, 1, 0, 0, 0, 0) == -1)
	/* signal QUIT or signal SIGUSR */
	if (terminated) break;
    /* signal SIGUSR */
	else if (timer_signal && !name_attached) {
	  ATTACH_NAME;
	  continue;
	}
    /* error */
	else msg(MSG_EXIT_ABNORM,"error reading from %s",basename(argv[optind]));
      else if (buf.sync == FRAME_CHAR) {
	msg(MSG_DEBUG,"found frame");
	break;
      }

    i = fr ? FRAME_SIZE-1 : FRAME_SIZE;
    if (terminated) break;
    else if (timer_signal && !name_attached) ATTACH_NAME;
	
    /* then read rest of frame */
    while (dev_read(fd, fr ? (char *)&buf.fr : (char *)&buf, i, i, 0, 0, 0, 0) == -1)
      if (terminated) break;
      else if (timer_signal && !name_attached) {
	ATTACH_NAME;
	continue;
      }
      else msg(MSG_EXIT_ABNORM,"error reading from %s",basename(argv[optind]));

    if (terminated) break;
	
    /* check frame out */
    fr=1;
    if (buf.sync != FRAME_CHAR) {
      msg(MSG_DEBUG,"bad frame: sync");
      bad_frames++;
      continue;
    }
    swab((char *)&buf.crc,(char *)&buf.crc,2);
    if ( (c=check_crc((unsigned char *)&buf.fr,(unsigned long)sizeof(PIF_FRAME))) != buf.crc) {
      msg(MSG_DEBUG,"bad frame: crc");
      bad_frames++;
      msg(MSG_DBG(1),"Bad CRC: %X, should be %X",buf.crc,c);
      continue;
    }
    fr = 0;

    /* valid first frame is available, disable timeout */
    if (timeout > 0) {
      rmtimer(timer_id);
      signal(SIGUSR1,SIG_DFL);
      timeout = -1;
    }

    if (timer_signal && !name_attached) ATTACH_NAME;

    /* get current system time */
    timesecs = time(NULL);
    timestruct = gmtime(&timesecs);

    /* settings on first frame */
    if (first_frame) {
      msg(MSG,"GPS Time from 1st PIF frame: %d:%d:%d",buf.fr.u.GPStime.hrs, buf.fr.u.GPStime.mins, buf.fr.u.GPStime.secs);
      first_frame=0;
    }
	
    last_jd = jd;
    jd = buf.fr.u.GPStime.hrs;	
    if (jd < last_jd) {
      jd_i++;		
      msg(MSG,"Day %d of Experiment",jd_i);
    }	
    last_dom = dom;
    dom = timestruct->tm_mday;
    if (dom != last_dom) dom_i++;
		
    /* compare system time and pif time */
    sys_secs_from_mid = (long)timestruct->tm_hour*60*60+(long)timestruct->tm_min*60+(long)timestruct->tm_sec;
    pif_hrs = buf.fr.u.GPStime.hrs;
    pif_mins = buf.fr.u.GPStime.mins;
    pif_secs = buf.fr.u.GPStime.secs;
    pif_secs_from_mid = (long)pif_hrs*60*60+(long)pif_mins*60+(long)pif_secs;
    /* allow for day changes */
    pif_secs_from_mid += (jd_i * 86400);
    sys_secs_from_mid += (dom_i * 86400);
		
    if (terminated) break;
	
    /* set system clock on first frame */
    if (set_clock > 0) {
      /* substitute in hours and seconds */
      timestruct->tm_hour = pif_hrs;
      timestruct->tm_min = pif_mins;
      timestruct->tm_sec = pif_secs;
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
	sys_secs_from_mid = pif_secs_from_mid;
      }
      set_clock = -1;
      if (quitter) terminated = 1;
      else  ATTACH_NAME;
    }

    temp = pif_secs_from_mid - sys_secs_from_mid;
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
      if (!(Col_set_pointer(PIF_FLAG_ID, &buf.fr, 0))) {
	with_dg = 1;
	msg(MSG,"achieved cooperation with DG");
      } else
	if (quit_no_dg) msg(MSG_EXIT_ABNORM,"Can't cooperate with DG");
	else {
	  errno = 0;
	  set_response(NLRSP_QUIET); /* for norts Col_set_pointer */
	}

    wd = temp;

    /* correct byte order */
    swab((char *)&buf.fr.AltB,(char *)&buf.fr.AltB,2);
    swab((char *)&buf.fr.AltG,(char *)&buf.fr.AltG,6);
    buf.fr.u.TDrft = wd;
    outbuf=buf.fr;

    msg(MSG_DEBUG,"Time Deviation: %d",wd);
    msg(MSG_DEBUG,"Roll Angle: %X",	outbuf.RollA);
    msg(MSG_DEBUG,"Pitch Angle: %X",	outbuf.PtchA);
    msg(MSG_DEBUG,"Pitch Rate: %X",	outbuf.PtchR);
    msg(MSG_DEBUG,"Yaw Rate: %X",	outbuf.YawR);
    msg(MSG_DEBUG,"Attack Angle: %X",	outbuf.AttA);
    msg(MSG_DEBUG,"Sideslip Angle: %X",	outbuf.SideA);
    msg(MSG_DEBUG,"Accel Z: %X",	outbuf.AcclZ);
    msg(MSG_DEBUG,"Indicated Airspeed: %X",	outbuf.IndAS);
    msg(MSG_DEBUG,"Altitude (Baro): %X",	outbuf.AltB);
    msg(MSG_DEBUG,"Vertical Speed: %X",	outbuf.VertS);
    msg(MSG_DEBUG,"Heading: %X",	outbuf.Head);
    msg(MSG_DEBUG,"Lattitude: %lX",	outbuf.Latt);
    msg(MSG_DEBUG,"Longitude: %lX",	outbuf.Lnttd);
    msg(MSG_DEBUG,"Altitude (GPS): %X",	outbuf.AltG);
    msg(MSG_DEBUG,"Groundspeed: %X",	outbuf.GndS);
    msg(MSG_DEBUG,"Course: %X",	outbuf.Cours);
    msg(MSG_DEBUG,"True Airspeed: %X",	outbuf.TruAS);
    msg(MSG_DEBUG,"Ambient Temperature: %X",	outbuf.AmbT);
    msg(MSG_DEBUG,"Wind Speed: %X",	outbuf.WindS);
    msg(MSG_DEBUG,"Wind Direction: %X",	outbuf.WindD);

  }				/* while */
    
  /* summary statistics */
  msg(bad_frames ? MSG_WARN : MSG,"Bad frames: %u",bad_frames);
  msg(MSG,"Good Frames: %u",good_frames);
  DONE_MSG;

}				/* main */
