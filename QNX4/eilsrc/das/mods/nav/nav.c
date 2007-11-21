/*
    Nav Program: Reads navigation parameters on the ER2 and U2 aircraft
    from an asynchronous serial port at 9600 baud, 8 data bits, no parity,
    and 1 stop bit.
    Written by Eil 10/92.
    Upgraded to new Ascii format by Eil 4/18/95.
    Shares same data with collection as always.
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
#include <sys/dev.h>
#include <sys/name.h>
#include "das.h"
#include "eillib.h"
#include "nortlib.h"
#include "collect.h"
#include "globmsg.h"
#include "nav.tmc"

/* defines */
#define HDR "nav"
#define NAV HDR
/* a timeout flag, seconds and a default, valid when C flag given */
/* a quit flag, valid when C flag given, quit after time set or timeout */
#define OPT_MINE "CU:qQ"
#define FRAME_CHAR 0x01
#define CARRT 0x0D
#define LNFD 0x0A
#define SPACER 0x20
#define NAV_TIMEOUT 10
#define ATTACH_NAME \
{ \
    if (qnx_name_attach(getnid(),LOCAL_SYMNAME(NAV))==-1) \
      msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",NAV); \
    name_attached = 1; \
}

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE OPT_CC_INIT OPT_SERIAL_INIT;
static Nav_frame buf;
int terminated;
int timer_signal;
int set_clock;
int quitter;
send_id nav_send;
int gps_valid, last_gps_valid;
unsigned int bad_frames, good_frames;


/* functions */
int gd_frame(char b[]) {
  int frms;
  int i,j;
/*  char *hexs;*/

  frms = bad_frames + good_frames;
  
  if (b[0]!=FRAME_CHAR) return 0;
/*
  if (frms<20) {
    msg(MSG,"Nav Frame %d",frms);
    for (j=0;j<sizeof(Nav_frame);) {
      hexs=(char *)malloc(33*2+1);
      for (i=0;i<33 && j<sizeof(Nav_frame);i++,j++)
	sprintf(hexs+(i*2),"%02x",b[j]);
      msg(MSG,"%s",hexs);
      free(hexs);
    }
  }
*/
  last_gps_valid = gps_valid;
  gps_valid = (b[1]=='G') ? 1 : ((b[1]=='N') ? 0 : -1);

  if (gps_valid<0) {
    msg(MSG_DEBUG,"GPS Valid flag invalid");
    return 0;
  }

/*  msg(MSG_DBG(1),"Buffer[331] is %02x",b[331]); */

/*  switch (b[331]) {
  case LNFD:
  case CARRT: break;
  default: msg(MSG_DEBUG,"Frame Ending Char 331 Not Valid: %02x",b[331]);
    return 0;
  }

  switch (b[332]) {
  case LNFD:
  case CARRT: break;
  default: msg(MSG_DEBUG,"Frame Ending Char 332 Not Valid: %02x",b[332]);
    return 0;
  }
*/

  if (b[100]!=' ') {
    msg(MSG_DEBUG,"frame[100] invalid");
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
  int quit_no_dg;
  int jd, last_jd;
  int bad;
	
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
  jd = -1;
  gps_valid = -2;
  nav_send = NULL;

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
      msg(MSG_WARN,"option U not valid without option C: ignored");
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

  buf.header[0] = !FRAME_CHAR;
  fr = 1;

  /* up timer */
  if (timeout > 0) {
    signal(SIGUSR1,tim_signalfunction);
    reltimer( timer_id,  &timer, NULL);
  }

  set_response(NLRSP_WARN);	/* for norts Col_send_init */

  while (!terminated) {

    while (fr)
      /* find the 1st frame char */
      if (dev_read(fd, &buf.header[0], 1, 1, 0, 0, 0, 0) == -1)
	/* signals ? */
	if (terminated) break;
	else if (timer_signal) {
	  if (!name_attached)
	    ATTACH_NAME;
	  continue;
	}
        /* error */
	else
	  msg(MSG_EXIT_ABNORM,"error reading from %s",basename(argv[optind]));
      else
	if (buf.header[0]==FRAME_CHAR) {
	  msg(MSG_DEBUG,"found frame");
	  break;
	}
			
    i = fr ? 332 : 333;
    if (terminated) break;
    else if (timer_signal && !name_attached) ATTACH_NAME;
		
    /* then read rest of frame */
    while (dev_read(fd,fr ? &buf.header[1] : &buf.header[0],i,i,0,0,0,0) == -1)
      if (terminated) break;
      else if (timer_signal) {
	if (!name_attached)
	  ATTACH_NAME;
	continue;
      }
      else msg(MSG_EXIT_ABNORM,"error reading from %s",basename(argv[optind]));
	
    if (terminated) break;
	
    if (timer_signal && !name_attached) ATTACH_NAME;
	
    /* check whole frame out */
    if (!gd_frame((char *)&buf)) {
      fr = 1;
      buf.header[0] = !(FRAME_CHAR);
      msg(MSG_DEBUG,"bad frame: invalid frame chars");
      bad_frames++;
      continue;
    }

    /* got a good frame */
    if (!name_attached && (timeout==-1 || set_clock==0)) ATTACH_NAME;
    fr = 0;
    good_frames++;		
    /* get current system time */
    timesecs = time(NULL);
    timestruct = gmtime(&timesecs);
	
    if (last_gps_valid != gps_valid)
      if (gps_valid>0) msg(MSG,"Global Positioning System valid");
      else msg(MSG,"Global Positioning System INvalid");

    if (terminated) break;
    if (gps_valid>0) {
      last_jd = jd;
      buf.time[3]='\000';
      buf.time[6]='\000';
      buf.time[9]='\000';
      buf.time[12]='\000';
      jd = atoi(buf.time);
      if (jd != last_jd)
	msg(MSG,"Julian Date: %d",jd);
      /* set system clock */
      if (set_clock > 0) {
	/* substitute in hours and seconds */
	timestruct->tm_hour = atoi(&buf.time[4]);
	timestruct->tm_min = atoi(&buf.time[7]);
	/* add the one second delay mentioned in Experimenters handbook */
	timestruct->tm_sec = atoi(&buf.time[10]) + 1;
	if (set_clock > 1) {
	  /* figure day of month and months since Jan from Julian day */
	  for (i=0;i<12;i++) {
	    j = jd;
	    switch (i) {
	    case 1:		/* feb */
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
	if ( (clock_settime(CLOCK_REALTIME,&tp)) == -1)
	  msg(MSG_WARN,"Can't set clock");
	else {
	  msg(MSG,"setting system clock to %.24s %s",
	      ctime(&tp.tv_sec),
	      localtime(&tp.tv_sec)->tm_isdst ? tzname[1] : tzname[0]);
	}
	set_clock = -1;
	if (quitter) terminated = 1;
	else ATTACH_NAME;
      }
      buf.time[3]=':';
      buf.time[6]=':';
      buf.time[9]=':';
      buf.time[12]=' ';
    }

    if (nav_send==NULL)
      if ((nav_send=Col_send_init("N_data",&buf,sizeof(Nav_frame)))==NULL)
	if (quit_no_dg)
	  msg(MSG_EXIT_ABNORM,"Can't cooperate with DG");
	else {
	  errno = 0;
	  set_response(NLRSP_QUIET); /* for norts Col_send_init */
	}
      else msg(MSG,"achieved cooperation with DG");
		
    if (nav_send) Col_send(nav_send);
    if (gps_valid>0) {
      buf.time[12] = '\000';
      msg(MSG_DEBUG,"frame time is %s",buf.time);
     }
    buf.lat[9] = '\000';
    buf.lon[10] = '\000';
    msg(MSG_DEBUG,"frame position is %s %s",buf.lat,buf.lon);


  } /* while */

  if (timeout > 0) {
    rmtimer(timer_id);
    timeout = -1;
  }

  /* summary statistics */
  msg(bad_frames ? MSG_WARN : MSG,"Bad Frames: %u",bad_frames);
  msg(MSG,"Good Frames: %u",good_frames);
  DONE_MSG;

} /* main */
