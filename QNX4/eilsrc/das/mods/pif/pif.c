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
#include <sys/dev.h>
#include <sys/kernel.h>
#include <sys/timers.h>
#include <sys/name.h>
#include "das.h"
#include "eillib.h"
#include "nortlib.h"
#include "collect.h"
#include "globmsg.h"
#include "pif.h"

/* defines */
#define HDR "pif"
#define PIF HDR
#define OPT_MINE "CU:qQ"
#define FRAME_CHAR 0x05
#define PIF_TIMEOUT 10
#define ATTACH_NAME \
{ \
    if (qnx_name_attach(getnid(),LOCAL_SYMNAME(PIF))==-1) \
      msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",PIF); \
    name_attached = 1; \
}

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE OPT_CC_INIT OPT_SERIAL_INIT;
static Pif_frame buf;
int terminated;
int timer_signal;
int set_clock;
int quitter;
send_id pif_send;
unsigned int bad_frames, good_frames;

/* functions */
int gd_frame(Pif_frame *b) {
  unsigned short my_crc;
  
  if (b->pif_sync!=FRAME_CHAR) {
    msg(MSG_DEBUG,"Lost synch");
    return 0;
  }
  my_crc=check_crc(b,sizeof(Pif_frame)-sizeof(UBYTE2));
  if (my_crc!=b->pif_crc) {
    msg(MSG_DEBUG,"Frame rejected: bad CRC: computed: %u, frame: %u",
	my_crc,b->pif_crc);
    return 0;
  }
  return 1;
}

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
  int fd;
  timer_t timer_id;
  struct timespec tp;
  struct tm *timestruct;
  struct itimerspec timer;
  struct itimercb timercb;
  time_t timesecs;
  long timeout;
  int name_attached;
  int with_dg;
  int quit_no_dg;

  /* initialise msg options from command line */
  signal(SIGQUIT,my_signalfunction);
  signal(SIGINT,my_signalfunction);
  signal(SIGTERM,my_signalfunction);	
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;

  /* initialisations */
  terminated = 0;
  timer_signal = 0;
  set_clock = 0;
  quitter = 0;
  timeout = -1;
  name_attached = 0;
  quit_no_dg = 0;
  bad_frames = 0;
  good_frames = 0;

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
      msg(MSG_WARN,"option U not valid without option C: ignored");
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

  buf.pif_sync = !(FRAME_CHAR);
  fr = 1;

  /* up timer */
  if (timeout > 0) {
    signal(SIGUSR1,tim_signalfunction);
    reltimer( timer_id,  &timer, NULL);
  }

  set_response(NLRSP_WARN);	/* for norts Col_send_init */

  while (!terminated) {

    /* hunt for synch mode */
    while (fr)
      /* find 1 synch character */
      if (dev_read(fd, &buf.pif_sync, 1, 1, 0, 0, 0, 0) == -1)
	/* signals? */
	if (terminated) break;
	else if (timer_signal && !name_attached) {
	  ATTACH_NAME;
	  continue;
	}
        /* error */
	else
	  msg(MSG_EXIT_ABNORM,"error reading from %s",basename(argv[optind]));
      else if (buf.pif_sync == FRAME_CHAR) {
	msg(MSG_DEBUG,"found frame");
	break;
      }

    i = fr ? sizeof(Pif_frame)-1 : sizeof(Pif_frame);
    if (terminated) break;
    else if (timer_signal && !name_attached) ATTACH_NAME;
	
    /* then read rest of frame */
    while (dev_read(fd,fr ? (void *)(&buf.pif_sync)) :
		    (void *)(&buf),i,i,0,0,0,0) == -1)
      if (terminated) break;
      else if (timer_signal && !name_attached) {
	ATTACH_NAME;
	continue;
      }
      else msg(MSG_EXIT_ABNORM,"error reading from %s",basename(argv[optind]));

    if (terminated) break;

    /* check whole frame out */
    if (!gd_frame(&buf)) {
      fr = 1;
      buf.pif_sync = !(FRAME_CHAR);
      bad_frames++;
      continue;
    }

    fr = 0;
    good_frames++;

    /* correct byte order */
    swab((char *)&buf.pif_lat,(char *)&buf.pif_lat,4);
    swab((char *)&buf.pif_long,(char *)&buf.pif_long,4);
    swab((char *)&buf.pif_gs,(char *)&buf.pif_gs,2);
    swab((char *)&buf.pif_course,(char *)&buf.pif_course,2);
    swab((char *)&buf.pif_gps_alt,(char *)&buf.pif_gps_alt,2);
    swab((char *)&buf.pif_baro_alt,(char *)&buf.pif_baro_alt,2);
    swab((char *)&buf.pif_crc,(char *)&buf.pif_crc,2);

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

    if (terminated) break;
	
    /* set system clock on first frame */
    if (set_clock > 0) {
      /* substitute in hours and seconds */
      timestruct->tm_hour = buf.pif_time_hh;
      timestruct->tm_min = buf.pif_time_mm;
      timestruct->tm_sec = buf.pif_time_ss;

      /* set system clock */
      tp.tv_sec = mktime(timestruct);
      tp.tv_nsec = 0;
      if ( (clock_settime(CLOCK_REALTIME,&tp)) == -1)
	msg(MSG_WARN,"Can't set clock");
      else {
	msg(MSG,"setting system clock to %.24s %s",
	    ctime(&tp.tv_sec),
	    localtime(&tp.tv_sec)->tm_isdst ? tzname[1] : tzname[0]);
      }
      set_clock = -1;
      if (quitter) terminated = 1;
      else  ATTACH_NAME;
    }

    if (pif_send==NULL)
      if ((pif_send=Col_send_init("P_data",&buf,sizeof(Pif_frame)))==NULL)
	if (quit_no_dg)
	  msg(MSG_EXIT_ABNORM,"Can't cooperate with DG");
	else {
	  errno = 0;
	  set_response(NLRSP_QUIET); /* for norts Col_send_init */
	}
      else msg(MSG,"achieved cooperation with DG");
		
    if (pif_send) Col_send(pif_send);

    msg(MSG_DEBUG,"Indicated Airspeed: %X",	buf.pif_ias);
    msg(MSG_DEBUG,"Vertical Speed: %X",	buf.pif_vsi);
    msg(MSG_DEBUG,"Heading: %X",	buf.pif_hdg);
    msg(MSG_DEBUG,"Lattitude: %lX",	buf.pif_lat);
    msg(MSG_DEBUG,"Longitude: %lX",	buf.pif_long);
    msg(MSG_DEBUG,"Groundspeed: %X",	buf.pif_gs);
    msg(MSG_DEBUG,"Course: %X",	buf.pif_course);
    msg(MSG_DEBUG,"Altitude (GPS): %X",	buf.pif_gps_alt);
    msg(MSG_DEBUG,"Altitude (Baro): %X",	buf.pif_baro_alt);
    msg(MSG_DEBUG,"Roll Angle: %X",	buf.pif_roll);
    msg(MSG_DEBUG,"Pitch Angle: %X",	buf.pif_pitch);
    msg(MSG_DEBUG,"Pitch Rate: %X",	buf.pif_pitch_rate);
    msg(MSG_DEBUG,"Yaw Rate: %X",	buf.pif_yaw_rate);
    msg(MSG_DEBUG,"Attack Angle: %X",	buf.pif_alpha);
    msg(MSG_DEBUG,"Sideslip Angle: %X",	buf.pif_beta);
    msg(MSG_DEBUG,"Accel Z: %X",	buf.pif_accel_z);
    msg(MSG_DEBUG,"True Airspeed: %X",	buf.pif_tas);
    msg(MSG_DEBUG,"Ambient Temperature: %X",	buf.pif_ambtemp);
    msg(MSG_DEBUG,"Wind Speed: %X",	buf.pif_wind_speed);
    msg(MSG_DEBUG,"Wind Direction: %X",	buf.pif_wind_crs);
    msg(MSG_DEBUG,"Time: %u:%u:%u",buf.pif_time_hh,
	buf.pif_time_mm, buf.pif_time_ss);

  }				/* while */
    
  /* summary statistics */
  msg(bad_frames ? MSG_WARN : MSG,"Bad frames: %u",bad_frames);
  msg(MSG,"Good Frames: %u",good_frames);
  DONE_MSG;

}				/* main */
