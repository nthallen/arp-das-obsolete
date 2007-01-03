#include <i86.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <signal.h>
#include "msg.h"
#include "../lib/bo_proto.h"

#ifdef MY_DEBUG
#include <iostream.h>
#endif

/* global variables */
#define HDR "bodiag"
#define OPT_MINE "C:"

extern "C" {
char *opt_string=OPT_MINE OPT_MSG_INIT;
};
int terminated;

void my_signalfunction(int sig_number) {
  terminated = 1;
}

#ifdef MY_DEBUG
extern volatile short num;
extern volatile unsigned short d0;
extern volatile unsigned long d1;
extern volatile long d2;
extern volatile long d3;
extern volatile long d4;
extern volatile short n;
#endif

main(int argc, char **argv) {

  /* getopt variables */
  extern char *optarg;
  extern int optind, opterr, optopt;

  short code;
  struct BoGrams_status stat;
  float huge *d;
  int i,k,nb,npk;
  long j;
  long coadd;

  signal(SIGQUIT,my_signalfunction);
  signal(SIGINT,my_signalfunction);
  signal(SIGTERM,my_signalfunction);

  msg_init_options(HDR,argc,argv);

  coadd = 1L;

  /* process args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case 'C': coadd = atol(optarg); break;
    case '?': msg(MSG_FATAL,"Invalid option -%c",optopt);
    default: break;
    }
  } while (i!=-1);

  BEGIN_MSG;
  d = NULL;
  if ((d=(float huge *) halloc(32768L, sizeof(float))) == NULL) {
    msg(MSG_WARN,"Can't allocate %ld bytes",32768L * sizeof(float));
    goto end;
  }

  msg(MSG,"Initialising the Bomem Acquistion Subsystem");
  if ( (code = open(6,1,7,5,0x220,15799.7,524288L,NULL,30,0))) {
    msg(MSG_WARN,"Error Code for OPEN: %d",code);
    goto end;
  }

  while (!terminated) {
    msg(MSG,"Requesting Data: 1 Measurement of %d Scans Coadded", coadd);
    if ( (code = start(1,0,coadd,1,0,1,0,0,0,0,0))) {
      msg(MSG_WARN,"Error Code for START: %d",code);
      goto end;
    }
#ifdef MY_DEBUG
    do {

	cout << num << " <-num n-> " << n << "\n";
	cout << d0 << " " << d1 << " " << d2 << " " << d3 << " " << d4 <<"\n";
    } while (!terminated);
#endif
    do {
      msg(MSG,"Getting Status, Polling");
      if ( (code = get_status(&stat)) ) {
	msg(MSG_WARN,"Error Code for GET_STATUS: %d",code);
	goto end;
      }
      if (stat.done & 0x01) { /* Acquisition Not done */
	msg(MSG_DEBUG,"Time Left to Complete Acquisition: %f",stat.tl);
	delay(stat.tl*1000);
      } else if (stat.done & 0x04) { /* FIFO Overrun */
	msg(MSG_WARN,"FIFO Overrun");
	goto end;
      }
      else if (stat.done & 0x02) {	/* Acquisition Done */
	msg(MSG,"Getting Data");
	if ((code = get_data(0,&stat,32768L,(float huge *)d))) {
	  msg(MSG_WARN,"Error Code for GET_DATA: %d",code);
	  goto end;
	}
	msg(MSG_DEBUG,"Elapsed Time Since Start of Acquisition: %f",stat.et);
	msg(MSG_DEBUG,"Sampling Frequency: %f",stat.sx);
	msg(MSG_DEBUG,"No. of Points: Double Interferogram: %ld",stat.npts);
	msg(MSG_DEBUG,"Resolution in cm-1: %d",stat.res);
	msg(MSG_DEBUG,"No. of Samples per Fringe: %d",stat.sp);
	msg(MSG_DEBUG,"Positive ZPD: %f",stat.zpd_pos);
	msg(MSG_DEBUG,"Negative ZPD: %f",stat.zpd_neg);
	msg(MSG_DEBUG,"DOUBLE INTERFEROGRAM: %f %f %f %f %f ...",*d,*(d+1),
	    *(d+2), *(d+3), *(d+4));
	npk = 1024 / sizeof(float);
	nb = (stat.npts * sizeof(float)) / 1024;
	/* write in blocks of 1024 */
	for (k=0; k< nb; k++)
	     write(STDOUT_FILENO,d + (k * npk), 1024);
/*	for (j=0L;j<stat.npts;j++) printf("%f\n", *(d+j));*/
    } else {
	msg(MSG_WARN,"Unknown Done Status: %x",stat.done);
	goto end;
      }
    } while (!(stat.done & 0x02) && !terminated);
  }

 end:
  if (d) hfree(d);
  msg(MSG,"Shutting Down the Bomem Acquisition Subsystem");
  close();
  DONE_MSG;
  msg_end();
}



