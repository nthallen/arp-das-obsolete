/*
    DBR Data Generator Serial in main common module.
    Written Sep 1992 by Eil for QNX 4.
*/

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/timers.h>
#include <sys/proxy.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "globmsg.h"
#include "das.h"
#include "dbr.h"
#include "dbr_mod.h"
#include "nortlib.h"
#include "eillib.h"
#include "serin.h"
#include "rational.h"

/* defines */
#define HDR "sin"
#define OPT_MINE "Q"

/* global variables */
char *opt_string=OPT_DG_INIT OPT_DG_DAC_IN OPT_MSG_INIT OPT_CC_INIT OPT_SERIAL_INIT OPT_MINE;
char *minors;	    /* space for 3 minor frames */
int fd;		    /* descriptor to read from */
char *filename;	    /* filename of given file/device */
pid_t dev_proxy;    /* proxy signals when data ready */
pid_t tim_proxy;    /* timer proxy */
mode_t filetype;    /* from struct stat */
rational mfsec;	    /* minor frames per sec rational */
rational tr;	    /* for rowlets increment and decrement */
int limit;
int quitter;
unsigned int lost;

main( int argc, char **argv )  {
  /* getopt variables */
  extern char *optarg;
  extern int optind, opterr, optopt;
  
  /* local variables */		
  int i;
  struct itimercb tcb;
  struct itimerspec tval;
  timer_t timer;
  struct stat *st;
  rational rat;
  unsigned char hdr;
  
  /* initialise msg options from command line */
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;
  cc_init_options(argc,argv, DCT_TM, DCT_TM, 0, 0, FORWARD_QUIT);
  
  /* initialisations */
  quitter = 0;
  
  /* process command line args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
    case 'Q': quitter = 1; break;
      default : break;
    }
  }  while (i!= -1);
  
  if (optind >= argc) msg(MSG_EXIT_ABNORM,"no file/device specified");
  if (argc > optind+1) msg(MSG_EXIT_ABNORM,"only one file/device allowed");
  filename = basename(argv[optind]);
  
  if ( (fd = open(argv[optind],O_RDONLY)) == -1)
    msg(MSG_EXIT_ABNORM,"Can't open %s",filename);
  
  /* regular file or device (character special file) ? */
  st = (struct stat *)malloc(sizeof(struct stat));
  if ( (fstat(fd, st)) == -1)
    msg(MSG_EXIT_ABNORM,"Can't get status of %s",filename);
  filetype = st->st_mode;
  free(st);
  if (!(S_ISCHR(filetype)) && !(S_ISREG(filetype)))
    msg(MSG_EXIT_ABNORM,"%s is not a regular or character special file",filename);
  
  if ( S_ISCHR(filetype) ) serial_init_options(argc,argv,fd);
  hdr=SERIN_MSG_HDR;
  
  if ( (dev_proxy = qnx_proxy_attach(0, &hdr, 1, -1)) == -1)
    msg(MSG_EXIT_ABNORM,"Can't attach proxy");
  
  if ( (tim_proxy = qnx_proxy_attach(0, &hdr, 1, -1)) == -1)
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
  lost = 0;
  
  DG_operate();
  if (lost) msg(MSG_WARN,"Lost Synch %u times",lost);
  
  DONE_MSG;
}
