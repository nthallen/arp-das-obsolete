/*
    Topaz Program: Serial Control of the Topaz Series Power Supply.
    Asynchronous serial port parameters at 9600 baud, 8 data bits, no parity,
    and 1 stop bit.
*/

/* header files */
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/dev.h>
#include <sys/proxy.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include "das.h"
#include "eillib.h"
#include "nortlib.h"
#include "collect.h"
#include "globmsg.h"
#include "topaz.h"

/* defines */
#define HDR "paz"
#define OPT_MINE "QqCf:"
#define FRAME_CHAR ';'
#define COLON ':'
#define NULC '\000'
#define SPACE ' '
#define PAZ_SIZE 125
#define PAZ_TIMEOUT 3
/* topaz states */
#define NON_RESPONSIVE 0
#define SYNCHRONIZED 1
#define RESPONSIVE 2
#define UNKNOWN 3
/* end of topaz states */
#define CHECK_WRITE_CMD(BUF) \
  (isalpha(*BUF) && strchr(BUF,FRAME_CHAR) \
   && strchr(BUF,NULC) && !strchr(BUF,'?'))
#define DEV_NULLS2SPACES \
 { \
    for (i=0;i<bytes_read;i++) \
      if (dev_buf[i]==NULC) dev_buf[i]=SPACE; \
    dev_buf[bytes_read] = NULC; \
 }


/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE OPT_CC_INIT OPT_SERIAL_INIT;
pid_t arm_proxy, rem_arm_proxy;
struct termios termv;
struct _dev_info_entry info;
int terminated, alarmed, requested_frame, first;
int topaz_state;
int fd;
int bytes_read;
int semicolons_read;
send_id paz_send;
char d1_cur_setpt[5], d2_cur_setpt[5], d1_temp_setpt[5], d2_temp_setpt[5];
char d4_temp_setpt[5], rep_rate_setpt[6], diode_event[2], paz_status[17];
char last_status[17];
char *dev_buf, *cmd_buf;
unsigned int bad_frames, good_frames, ignored, part_frames, no_frames;
unsigned int bad_sends;
int synchs; /* non ordinary synchronisations */
static Paz_frame buf;
#define NUM_CODES 104
char *codestrings[NUM_CODES] = {
  "Wait", "Power Mode Ready", "Current Mode Ready", "Power Mode Adjust",
  "Current Mode Adjust", "Diode Off, Temperature Ready",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL,
  "EEPROM Data Read Error", "AC Fault > 50ms", "System Boot Marker",
  "Communications Transmission Error", "Laser Power Outside Ready Range",
  "Power Mode Adjust Timeout", "Passbank Over-temperature",
  "Both Passbanks Current Limited",
  "Diode Module Interlock Test: Bad Voltage",
  "Diode Module Interlock Test: Bad Logic",
  "Diode Module Safety Check 2: Bad Voltage",
  "Diode Module Safety Check 2: Bad Logic",
  "Diode Module Safety Check 1: Bad Voltage",
  "Diode Module Safety Check 1: Bad Logic",
  "Shutter Failed Interlock Safety Test",
  "Both Sensors 1 & 2 Failed Shutter Safety Test",
  "Sensor 2 Failed Shutter Safety Test",
  "Sensor 1 Failed Shutter Safety Test",
  "Safety Shutter Open, Should Be Closed",
  "Safety Shutter Closed, Should be Open",
  "EEPROM Data Not Available During Start-Up",
  "EEPROM Fault On Write Condition",
  "Bad Configuration for uP", "Compressor Failed Start-up",
  "Heater Failed Start-up Test", 
  "Shorted Thermistor 2", "Open Thermistor 2",
  "Shorted Thermistor 1", "Open Thermistor 1",
  "Multiple Errors", "Diode Over Temperature (>3.7C)",
  "Diode Under Temperature (<7.5C)",
  "Current Limit Passbank 2 Active", "Current Limit Passbank 1 Active", 
  "Interlock Active",
  "Safety Relay for Diode 2 is Closed, Should be Open",
  "Safety Relay for Diode 2 is Open, Should be Closed",
  "Safety Relay for Diode 1 is Closed, Should be Open",
  "Safety Relay for Diode 1 is Open, Should be Closed",
  "Rod tower over temperature (>40 C)",
  "Rod tower under temperature (<15 C)",
  "Open rod tower thermistor",
  "Shorted rod tower thermistor"
  };

void my_signalfunction(int sig_number) {
  terminated = 1;    
}

void initialise_setpoints(void) {
  strnset(d1_cur_setpt,NULC,5);
  strnset(d2_cur_setpt,NULC,5);
  strnset(d1_temp_setpt,NULC,5);
  strnset(d2_temp_setpt,NULC,5);
  strnset(d4_temp_setpt,NULC,5);
  strnset(rep_rate_setpt,NULC,6);
  strnset(diode_event,NULC,2);
  strnset(last_status,NULC,17);
}

void my_alarmfunction(int sig_number) {
  alarmed = 1;
}

void alarm_myself(void) {
  alarmed = 0;
  signal(SIGALRM, my_alarmfunction);
  alarm(PAZ_TIMEOUT);
}

void disalarm_myself(void) {
  alarm(0);
  alarmed = 0;
}

/* Info Frame Request, 13 pieces of data, receive frame size = 122 */
#define INFO_FRAME_SIZE 122
void Info_Request(char *s) {
  if (!requested_frame) {
    if (write(fd,
      "?v;?X11;?X12;?X21;?X22;?X31;?X32;?X41;?X42;?X51;?X52;?X61;?X62;",63)
	!= 63 && errno!=EINTR)
      msg(MSG_FAIL,"Error %d writing to %s", errno, s);
    requested_frame = 1;
  }
}

/* Collection Frame Request, 7 pieces of data, receive frame size > 40 */
#define COL_FRAME_SIZE 40
void Col_Request(char *s) {
  if (!first)
    if (!requested_frame) {
      if (write(fd,"?C1;?C2;?T1;?T2;?X61;?X62;?H;",29) != 29 && errno!=EINTR)
	msg(MSG_FAIL,"Error %d writing to %s", errno, s);
      requested_frame = 1;
      msg(MSG_DBG(1),"Requested Collection Frame from Topaz");
    } else ignored++;
}

/* return 0 if program receives terminating signal */
int Synchronise(char *s) {
  int i, iterations;
  char c[50];
  int first = 0; /* the first sychronisation can continue until success */

  disalarm_myself();
  msg(MSG_DEBUG,"Synchronising with Topaz");
  if (topaz_state == SYNCHRONIZED) synchs++;
  if (topaz_state == UNKNOWN) first = 1;

  /* disarm the device */
  if (arm_proxy) {
    dev_arm(fd, _DEV_DISARM,_DEV_EVENT_INPUT);
    /* Get outstanding arming msgs */
    do {
      if ( (i=Creceive(arm_proxy,0,0)) == -1)
	if (errno!=ENOMSG && errno!=EINTR)
	  msg(MSG_FATAL,"Error Receiving from Arming Proxy %d",arm_proxy);
    } while (i>0);
    errno = 0;
  }

  if (terminated) return(0);

  if ( arm_proxy == 0 ) {
	/* set up device arming proxy */
	if ( (arm_proxy = qnx_proxy_attach(0, 0, 0, -1)) == -1)
	  msg(MSG_FATAL,"Can't make arming proxy");

	/* set up virtual proxy */
	if ( (rem_arm_proxy=qnx_proxy_rem_attach(info.nid,arm_proxy)) == -1)
	  msg(MSG_FATAL,"Error getting remote proxy on Node %d",info.nid);
	msg(MSG_DBG(1),"Arming Proxies Attached");

	if (terminated) return(0);

	/* set hardware flow control */
	if (tcgetattr(fd, &termv) == -1)
	  msg(MSG_FATAL,"Can't get terminal control settings for %s",s);
	termv.c_cflag |= (OHFLOW);
	termv.c_cflag |= (IHFLOW);
	if (tcsetattr(fd,TCSANOW,&termv) == -1)
	  msg(MSG_FATAL,"Can't set terminal control settings for %s",s);
	msg(MSG_DBG(1),"Added Hardware Flow Control to %s",s);

	if (terminated) return(0);
  }

#ifdef EXTENSIVE_RESET
  sprintf(c,"stty +reset < %s",s);
#endif
  iterations = 0;

  do {
    /* rid flow paged */
    if (tcflow(fd,TCOONHW | TCIONHW) == -1)
      msg(MSG_WARN,"Can't Set Paged Flow to OFF");
    msg(MSG_DBG(1),"Set Paged Flow State Off");
    /* Synchronise Hardware and Software settings, reset hardware, drastic */
#ifdef EXTENSIVE_RESET
    i = system(c);
    msg(MSG_DBG(1),"Reset Hardware: %s: result %d",c,WEXITSTATUS(i));
#endif
    /* Clear Topaz Buffers */
    if (write(fd,"\003",1)!=1 && errno!=EINTR)
      msg(MSG_FATAL,"Error writing to %s",s);
    msg(MSG_DBG(1),"Cleared Topaz Buffers");
#ifdef EXTENSIVE_RESET
    sleep(1);
#endif
    /* flush */
    tcflush(fd, TCIOFLUSH);
    msg(MSG_DBG(1),"Flushed %s",s);

    /* kick and read */
    if (write(fd,"?v;",3)!=3 && errno!=EINTR)
      msg(MSG_FATAL,"Error writing to %s",s);
    strnset(dev_buf,NULC,PAZ_SIZE);
    bytes_read = 0;
    do {
      i=dev_read(fd, dev_buf+bytes_read, PAZ_SIZE-bytes_read-1,
		  20, 3, PAZ_TIMEOUT*10, 0, 0);
      switch (i) {
      case 0:
	if ( bytes_read == 0 ) {
	  if (topaz_state != NON_RESPONSIVE)
	    msg(MSG_WARN,"No Response from Topaz");
	  topaz_state = NON_RESPONSIVE;
	}
	break;
      case -1:
	if (errno !=EINTR) msg(MSG_FATAL,"Error reading from %s",s);
	break;
      default:
	bytes_read += i;
	if ( dev_buf[bytes_read-1] == FRAME_CHAR ) {
	  topaz_state = SYNCHRONIZED;
	  first = 0;
	} else {
	  topaz_state = RESPONSIVE;
	  dev_buf[bytes_read] = NULC;
	  msg( MSG_DBG(1), "Synch saw %d:%s:", bytes_read, dev_buf );
	}
	break;
      }
    } while ( i > 0 );
    iterations++; /* there is a limit to everything */
  } while (!terminated &&
      (first || (topaz_state==RESPONSIVE && iterations<4)));
  if (terminated) return(0);
  
  /* arm the device */
  if (dev_arm(fd, rem_arm_proxy,_DEV_EVENT_INPUT)==-1)
    msg(MSG_FATAL,"Can't arm device %s",s);

  bytes_read = 0;
  semicolons_read = 0;
  requested_frame = 0;
  if (topaz_state == SYNCHRONIZED)
    msg(MSG_DEBUG,"Synchronisation completed successfully with Topaz");
  alarm_myself();
  return(1);
}

/* Handle first info frame asked for by this program from power supply */
void info_frame(void) {
/* "?v;?X11;?X12;?X21;?X22;?X31;?X32;?X41;?X42;?X51;?X52;?X61;?X62" */
  char *beg, *end;
  beg = dev_buf; end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Topaz Sofware Version: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 1: Serial Number: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 2: Serial Number: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 1: Ship Date: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 2: Ship Date: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 1: Operating Temperature: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 2: Operating Temperature: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 1: Current Limit: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 2: Current Limit: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 1: Initial Operating Current: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 2: Initial Operating Current: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 1: Operating Hours: %s",beg);
  beg = end+1;   end = strpbrk(beg,";"); *end = NULC;
  msg(MSG,"Diode 2: Operating Hours: %s",beg);
}

/* return 1 on success, which is only if a number of characters
   followed by the units string and a semicolon is less than length
*/
int paz_element( char *dest, int length, char *units, int *offset ) {
  char *p;
  int i;

  p = dev_buf+(*offset);
  for (i = 0; i < length-1; i++) {
    if ( p[i] == ';' ) break;
    dest[i] = p[i];
  }
  if ( p[i] == ';' ) {
	dest[i] = '\0';
	*offset += i+1;
	i -= strlen(units);
	if ( i > 0 && strcmp(units, dest+i) == 0 )
	  return 1;
  }
  msg(MSG_DEBUG, "Frame rejected at offset %d", *offset);
  return 0;
}

#ifdef LOGSOMERECORDS
  FILE *logfile = NULL;
  long int n_records = 0;
#endif

/* constructs the structure for collection */
int make_Paz_frame(int have_status, char **histptr) {
  /* dev_buf already checked for correct number of semi-colons */
  int offset;

#ifdef LOGSOMERECORDS
  if ( logfile == 0 && n_records == 0 ) {
    logfile = fopen( "topazlog.log", "w" );
	if ( logfile == 0 ) msg( 2, "Unable to open log file" );
  }
  if ( logfile != 0 && n_records >= 10 ) {
	fclose( logfile );
	logfile = NULL;
  }
  n_records++;
  if ( logfile != 0 ) {
    fprintf( logfile, "Record %ld [%d]:\n", n_records, bytes_read );
	fwrite( dev_buf, bytes_read, 1, logfile );
	fprintf( logfile, "\n" );
  }
#endif

  offset = 0;
  if (
      paz_element( buf.d1_cur, 7, "A", &offset ) &&
      paz_element( buf.d2_cur, 7, "A", &offset ) &&
      paz_element( buf.d1_temp, 7, "C", &offset ) &&
      paz_element( buf.d2_temp, 7, "C", &offset ) &&
      paz_element( buf.d1_op_hrs, 10, "hrs", &offset ) &&
      paz_element( buf.d2_op_hrs, 10, "hrs", &offset ) ) {
    if ( have_status ) {
      if ( offset+16 < bytes_read && dev_buf[offset+16] == ';' ) {
	dev_buf[offset+16] = '\0';
	memcpy(buf.paz_status,dev_buf+offset,17); /* History Status Array */
	*histptr = dev_buf+offset;
      } else return 0;
    }
    strcpy(buf.d1_cur_setpt,d1_cur_setpt);   /* Diode 1 current setpoint */
    strcpy(buf.d2_cur_setpt,d2_cur_setpt);   /* Diode 2 current setpoint */
    strcpy(buf.d1_temp_setpt,d1_temp_setpt); /* Diode 1 temperature setpoint */
    strcpy(buf.d2_temp_setpt,d2_temp_setpt); /* Diode 2 temperature setpoint */
    strcpy(buf.d4_temp_setpt,d4_temp_setpt); /* Doubler temperature setpoint */
    strcpy(buf.rep_rate_setpt,rep_rate_setpt);/* Rep Rate setpoint */
    strcpy(buf.diode_event,diode_event);     /* Diodes On/Off Event Queued */
    msg(MSG_DBG(1),"Diode 1 Current: %s",buf.d1_cur);
    msg(MSG_DBG(1),"Diode 2 Current: %s",buf.d2_cur);
    msg(MSG_DBG(1),"Diode 1 Temperature: %s",buf.d1_temp);
    msg(MSG_DBG(1),"Diode 2 Temperature: %s",buf.d2_temp);
    msg(MSG_DBG(1),"Diode 1 Operating Hours: %s",buf.d1_op_hrs);
    msg(MSG_DBG(1),"Diode 2 Operating Hours: %s",buf.d2_op_hrs);
    if (strlen(d1_cur_setpt))
      msg(MSG_DBG(1),"Diode 1 Current SetPoint: %s",d1_cur_setpt);
    if (strlen(d2_cur_setpt))
      msg(MSG_DBG(1),"Diode 2 Current SetPoint: %s",d2_cur_setpt);
    if (strlen(d1_temp_setpt))  
      msg(MSG_DBG(1),"Diode 1 Temperature SetPoint: %s",d1_temp_setpt);
    if (strlen(d2_temp_setpt))  
      msg(MSG_DBG(1),"Diode 2 Temperature SetPoint: %s",d2_temp_setpt);
    if (strlen(d4_temp_setpt))  
      msg(MSG_DBG(1),"Doubler Temperature SetPoint: %s",d4_temp_setpt);
    if (strlen(rep_rate_setpt))  
      msg(MSG_DBG(1),"Rep Rate SetPoint: %s",rep_rate_setpt);
    if (strlen(diode_event))  
      msg(MSG_DBG(1),"Diode Event Queued: %s",diode_event);
    if (have_status) {
      char *beg = dev_buf+offset;
      msg(MSG_DBG(1),
	"Status History Array: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
	beg[0],beg[1],beg[2],beg[3],beg[4],beg[5],beg[6],beg[7],beg[8],beg[9],
	beg[10],beg[11],beg[12],beg[13],beg[14],beg[15]);
    }
    return 1;
  } else return 0;
}

void handle_status(char *status) {
  char *p;
  int i,j;
  unsigned int k;
  if (status==NULL) {
    msg(MSG_WARN,"Software Error: No Status");
    return;
  }
  p = status; i=16;
  while (i && memcmp(p++,last_status,i)) i--;
  for (i=16-i, j=i-1; j>=0; j--) {
    k = *(status+j);
    if (k > (NUM_CODES-1)) msg(MSG_WARN,"Status Code Out of Range: %u",k);
    else if (codestrings[k] == NULL)
      msg(MSG_WARN,"Undefined Status Code: %u",k);
    else msg(k>=50 ? MSG_WARN : MSG,"Topaz Status: %s",codestrings[k]);
	if ( k == 63 ) /* System Boot Marker */
	  initialise_setpoints();
  }
  memcpy(last_status,status,17);
}

void main(int argc, char **argv) {

  /* getopt variables */
  extern char *optarg;
  extern int optind, opterr, optopt;

  /* local variables */
  int quit_no_dg, i, armed, name_id, test_mode;
  pid_t cc_id, col_proxy, test_proxy, from;
  char *p, r, cmd_file[FILENAME_MAX];
  FILE *cf;
  timer_t timer_id;
  struct itimerspec timer;
  struct sigevent event;

  /* initialise msgs and signals */
  signal(SIGQUIT,my_signalfunction);
  signal(SIGINT,my_signalfunction);
  signal(SIGTERM,my_signalfunction);
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;
  msg(MSG_DEBUG,"My Code Version: Dont Block on Topaz Power Off");

  /* var initialisations */
  col_proxy = test_proxy = arm_proxy = rem_arm_proxy = 0;
  bad_frames = good_frames = part_frames = no_frames = 0;
  topaz_state = UNKNOWN;
  bad_sends = 0;
  terminated = 0;
  bytes_read = 0;
  semicolons_read = 0;
  test_mode = 0;
  cc_id = from = 0;
  quit_no_dg = 0;
  synchs = 0;
  ignored = 0;
  armed = 0;
  first = 0;
  name_id = 0;
  timer_id = 0;
  fd = -1;
  cmd_file[0] = NULC;
  cf = NULL;
  initialise_setpoints();
  if ( (dev_buf = malloc(PAZ_SIZE)) == NULL)
    msg(MSG_FATAL,"Can't allocate %d bytes of data",PAZ_SIZE);
  if ( (cmd_buf = malloc(MAX_MSG_SIZE)) == NULL)
    msg(MSG_FATAL,"Can't allocate %d bytes of data",MAX_MSG_SIZE);

  /* process args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case 'Q': quit_no_dg = 1; break;
    case 'C': test_mode = 1; break;
    case 'f': strncpy(cmd_file, optarg, FILENAME_MAX-1); break;
    case '?': msg(MSG_FATAL,"Invalid option -%c",optopt);
    default: break;
    }
  } while (i!=-1);

  if (optind >= argc) msg(MSG_FATAL,"no device specified");
  if (argc > optind+1) msg(MSG_FATAL,"only one device allowed");

  /* open optional command file */
  if (strlen(cmd_file)) 
    if ( (cf = fopen(cmd_file,"r")) == NULL)
      msg(MSG_FATAL,"Can't open file %s to read",cmd_file);

  /* setup communications parameters */
  if ( (fd = open(argv[optind],O_RDWR)) == -1)
    msg(MSG_FATAL,"Can't open %s",argv[optind]);
  serial_init_options(argc, argv, fd);

  /* set up virtual proxy */
  if (dev_info(fd, &info) == -1)
    msg(MSG_FATAL,"Error getting dev info for %s",argv[optind]);

  if (!Synchronise(argv[optind])) goto cleanup;

  /* set up test mode */
  if (test_mode) {
    msg(MSG,"Read Test Mode");
    /* set up test proxy */
    if ( (test_proxy = qnx_proxy_attach(0, 0, 0, -1)) == -1)
      msg(MSG_FATAL,"Can't make test proxy");
    event.sigev_signo = -test_proxy;
    if ((timer_id = timer_create(CLOCK_REALTIME, &event)) == -1)
      msg(MSG_FATAL,"Can't attach timer for test mode");
    timer.it_value.tv_sec = 2L;
    timer.it_value.tv_nsec = 0L;
    timer.it_interval.tv_sec = 1L;
    timer.it_interval.tv_nsec = 0L;
    timer_settime(timer_id, 0, &timer, NULL);
  }

  /* register with cmdctrl */
  cc_id = cc_init_options(argc,argv,0,0,0,0,SET_BREAK);

  set_response(NLRSP_WARN);	/* for norts Col_send_init */

  /* look for DG */
  if (!test_mode) {
    if ((paz_send=Col_send_init("Paz_data",&buf,PAZ_SIZE)) == NULL)
      if (quit_no_dg) msg(MSG_FATAL,"Can't cooperate with DG");
      else {
	msg(MSG_WARN,"Can't cooperate with DG");
	errno = 0;
	set_response(NLRSP_QUIET); /* for norts Col_send_init */
      }
    else msg(MSG,"achieved cooperation with DG");

    /* set up proxy for collection */
    if ( (col_proxy = Col_set_proxy(PAZ_PROXY_ID, NULL)) == -1) {
      msg(quit_no_dg ? MSG_FATAL : MSG_WARN, "Can't set proxy for collection");
      errno = 0;
      set_response(NLRSP_QUIET); /* for norts Col_send_init */
    }
  }

  if (terminated) goto cleanup;

  /* query the laser system for Info frame, set the ball rolling */
  Info_Request(argv[optind]);

  /* attach name and advertise services */
  if ( (name_id = qnx_name_attach(getnid(),LOCAL_SYMNAME(PAZ))) == -1)
    msg(MSG_FATAL,"Can't attach symbolic name for %s",PAZ);

  /* Service Request Receive Loop */
  first = 1;
  while (!terminated) {

    if (alarmed) {
      if (requested_frame) {
	if (bytes_read) {
	  part_frames++;
	  /* If we have everything except status, report anyway */
	  if ( semicolons_read == 6 && make_Paz_frame( 0, NULL) ) {
	    msg(MSG_DEBUG,"Reporting Everything except Status");
	    if (paz_send)
	      if (Col_send(paz_send)) {
		bad_sends++;
		msg(bad_sends < 3 ? MSG_WARN : MSG_DEBUG,
		    "Error Sending to Collection");
	      }
	  }
	  DEV_NULLS2SPACES;
	  msg(part_frames<3?MSG_WARN: MSG_DEBUG,
	      "Partial Frame Response from Topaz: Bytes Read %d: %s",
	      bytes_read,dev_buf);
	  /* This will go on to re-synchronise */
	}
	else no_frames++;
      } else {
	if (test_mode) msg(MSG_FATAL,"Test Mode Prompting Not Functioning");
	else 
	  msg(MSG_WARN,
	      "Nobody seems to be Prompting me to Collect data from Topaz");
      }
      if (!Synchronise(argv[optind])) continue;
      if (first) Info_Request(argv[optind]);
    }
    strnset(cmd_buf,NULC,MAX_MSG_SIZE);
    if ( (from = Receive(0, cmd_buf, MAX_MSG_SIZE)) == -1)
      if (errno != EINTR) msg(MSG_FATAL,"Error Receiving");
      else continue;

    if (from == arm_proxy) {
      disalarm_myself();
      /* read data from device */
      if (!bytes_read) strnset(dev_buf,NULC,PAZ_SIZE);
      else if (bytes_read < 0 || bytes_read > PAZ_SIZE-1) 
	msg(MSG_DEBUG,"Bytes_read corrupted: %d",bytes_read);
      else
	msg(MSG_DBG(1),"Read again, bytes read %d, some dev buf: %s",
	    bytes_read,dev_buf);
      if ( (i = dev_read(fd, dev_buf+bytes_read,
			 (unsigned)((PAZ_SIZE - 1) - bytes_read),0,
			 PAZ_TIMEOUT*10,0,0,0)) == -1) {
	if (errno != EINTR)
	  msg(MSG_FATAL,"Error reading from %s",argv[optind]);
	else continue; /* slayed or bad bad signal that ends prog anyway */
      } else {
	if (i==0)
	  msg(MSG_WARN,"Software Error: Read Zero Bytes from Topaz");
	{ int j = bytes_read;
	  bytes_read += i;
	  for ( ; j < bytes_read; j++ )
	    if ( dev_buf[j] == FRAME_CHAR )
	      semicolons_read++;
	}
	dev_buf[bytes_read] = NULC;
	/* clear and re-arm the device */
	dev_arm(fd, _DEV_DISARM,_DEV_EVENT_INPUT);
	if (dev_arm(fd, rem_arm_proxy,_DEV_EVENT_INPUT)==-1)
	  msg(MSG_FATAL,"Can't arm device %s",argv[optind]);
      }

      if (first) {
	if (semicolons_read >= 13) {
	  info_frame();
	  first = 0;
	  bytes_read = 0;
	  semicolons_read = 0;
	  requested_frame = 0;
	  /* Now Write control commands from optional command file */
	  if (cf) {
	    errno = 0;
	    while (fgets(cmd_buf, MAX_MSG_SIZE-1, cf)) {
	      if ( (p=strrchr(cmd_buf,'\n')) != NULL) *p = NULC;
	      if (CHECK_WRITE_CMD(cmd_buf)) {
		i = strlen(cmd_buf);
		msg(MSG_DEBUG,"Writing File Topaz Control Command: %s",
		    cmd_buf);
		if (write(fd,cmd_buf,i) != i && errno!=EINTR)
		  msg(MSG_FATAL,"Error writing to %s",argv[optind]);
	      }
	    }
	    if (errno) msg(MSG_WARN,"Error reading from file %s",cmd_file);
	    fclose(cf); cf = NULL;
	  }
	}
	else if ( bytes_read == PAZ_SIZE-1 ) {
	  bad_frames++;
	  DEV_NULLS2SPACES;
	  dev_buf[PAZ_SIZE-1] = NULC;
	  msg(bad_frames<3?MSG_WARN: MSG_DEBUG,"Bad Info Frame: %s",dev_buf);
	  if (!Synchronise(argv[optind])) continue;
	  Info_Request(argv[optind]);
	}
      }
      else {
	 /* bytes_read > COL_FRAME_SIZE) */
	if ( semicolons_read >= 7 || bytes_read == PAZ_SIZE-1 ) {
	  if ( semicolons_read == 7 && make_Paz_frame( 1, &p ) ) {
	    bytes_read = 0;
	    semicolons_read = 0;
	    requested_frame = 0;
	    good_frames++;
	    if (paz_send) 
	      if (Col_send(paz_send))
		msg(MSG_DEBUG,"Error Sending to Collection");
	    /* handle status */
	    handle_status(p);
	  }
	  else {
	    bad_frames++;
	    DEV_NULLS2SPACES;
	    msg(bad_frames<3?MSG_WARN: MSG_DEBUG,"Bad Col Frame: %s",dev_buf);
	    if (!Synchronise(argv[optind])) continue;
	  }	
	}
      }
      alarm_myself();
    } /* from == arm_proxy */
    else if (from == test_proxy || from == col_proxy) {
	Col_Request(argv[optind]);
    } 
    else {
      /* send control commands to Topaz power supply */
      r = DAS_OK;
      if (CHECK_WRITE_CMD(cmd_buf)) {
	i = strlen(cmd_buf);
	/* look for setpoints */
	if ( (p=strrchr(cmd_buf,'C')) != NULL && *(p+2) == COLON) {
	  if ( *(p+1) == '1') {
	    strncpy(d1_cur_setpt,p+3,4);
	    if ( (p=strrchr(d1_cur_setpt,FRAME_CHAR)) != NULL) *p = NULC;
	  }
	  else {
	    strncpy(d2_cur_setpt,p+3,4);
	    if ( (p=strrchr(d2_cur_setpt,FRAME_CHAR)) != NULL) *p = NULC;
	  }
	}
	if ( (p=strrchr(cmd_buf,'T')) != NULL && *(p+2) == COLON) {
	  if ( *(p+1) == '1') strncpy(d1_temp_setpt,p+3,3);
	  else if ( *(p+1) == '2') strncpy(d2_temp_setpt,p+3,3);
	  else strncpy(d4_temp_setpt,p+3,4);
	}
	if ( (p=strrchr(cmd_buf,'D')) != NULL) {
	  if ( *(p+1) == '0') diode_event[0] = '0';
	  else diode_event[0] = '1';
	}
	if ( (p=strrchr(cmd_buf,'Q')) != NULL && *(p+1) == COLON) {
	    strncpy(rep_rate_setpt,p+2,5);
	    if ( (p=strrchr(rep_rate_setpt,FRAME_CHAR)) != NULL) *p = NULC;
	}
	msg(MSG_DEBUG,"Writing Topaz Control Command: %s",cmd_buf);
	if (write(fd,cmd_buf,i) != i && errno!=EINTR) {
	  if (Reply(from,&r,sizeof(reply_type)) == -1)
	    msg(MSG_WARN,"error replying to process %d",from);
	  msg(MSG_FATAL,"error writing to %s",argv[optind]);
	}
      }
      else {
	/* Bad control command */
	r = DAS_UNKN;
	cmd_buf[MAX_MSG_SIZE-1] = NULC;
	if (strchr(cmd_buf,'?'))
	  msg(MSG_WARN,"User Topaz Read Commands not Allowed");
	else msg(MSG_WARN,"Syntax Error: Invalid Topaz Cmd: not sent: %s",
	    cmd_buf);
      }
      if (Reply(from,&r,sizeof(reply_type)) == -1)
	msg(MSG_WARN,"error replying to process %d",from);
    }
  } /* while */

  /* cleanup and report stats */
 cleanup:
  if (name_id) qnx_name_detach(0,name_id);
  if (paz_send) Col_send_reset(paz_send);
  if (col_proxy) Col_reset_proxy(PAZ_PROXY_ID);
  if (test_proxy) qnx_proxy_detach(test_proxy);
  if (timer_id) timer_delete(timer_id);
  if (rem_arm_proxy) qnx_proxy_rem_detach(info.nid,rem_arm_proxy);
  if (arm_proxy) qnx_proxy_detach(arm_proxy);
  if (fd > -1) close(fd);
  if (cf) fclose(cf);
  if (cmd_buf) free(cmd_buf); if (dev_buf) free(dev_buf);
  msg(ignored ? MSG_WARN : MSG,
  "Collection Requests Not Granted (due to topaz response times): %u",ignored);
  if (synchs>1) msg(MSG_WARN,"Re-Synchronisations with topaz: %d",synchs);
  msg(bad_sends ? MSG_WARN : MSG, "Send to Collection Failures: %u",bad_sends);
  msg(no_frames ? MSG_WARN : MSG,"Non Responses: %u",no_frames);  
  msg(part_frames ? MSG_WARN : MSG,"Partial frames: %u",part_frames);  
  msg(bad_frames ? MSG_WARN : MSG,"Bad frames: %u",bad_frames);
  msg(MSG,"Good Frames: %u",good_frames);
  DONE_MSG;
} /* main */
