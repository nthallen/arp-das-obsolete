/*
  Idler Program. Counts CPU idle time, can be interfaced with collection
  and cmdctrl.
  Time Accounting Must be Enabled by Operating System.
  There should be no other task set at priority 1 and running,
  like the clock program.
*/

/* header files */
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/times.h>
#include <sys/sched.h>
#include <sys/osinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/disk.h>
#include "das.h"
#include "eillib.h"
#include "nortlib.h"
#include "collect.h"
#include "globmsg.h"
#include "idler.h"

/* defines */
#define HDR "idle"
#define OPT_MINE "Q"
#define LOOPER 35000

/* global variables */
char *opt_string=OPT_MSG_INIT OPT_MINE OPT_CC_INIT;
int terminated;

/* functions */

void my_signalfunction(int sig_number) {
  terminated = 1;    
}

void main(int argc, char **argv) {

  /* getopt variables */
  extern char *optarg;
  extern int optind, opterr, optopt;

  /* local variables */
  struct timespec beg, end;
  float elapse;
  int quit_no_dg;
  struct sched_param p;
  char usage[3];
  struct tms cputim;
  struct _osinfo osdata;
  int i;
  int oldpri;
  char hi_mem;
  int fd;
  long free_b, tot_b;
  unsigned int l;

  /* initialise msg options from command line */
  signal(SIGQUIT,my_signalfunction);
  signal(SIGINT,my_signalfunction);
  signal(SIGTERM,my_signalfunction);
  msg_init_options(HDR,argc,argv);
  BEGIN_MSG;

  oldpri = getprio(0);
  p.sched_priority = 1;
  if ( (sched_setscheduler(0, SCHED_FIFO, &p)) == -1)
    msg(MSG_FATAL,"Can't set scheduling policy to SCHED_FIFO");

  /* initialisations */
  terminated = 0;
  quit_no_dg = 0;
  usage[0] = usage[1] = usage[2] = hi_mem = 0;
  free_b = tot_b = 0;

  /* process args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case 'Q': quit_no_dg = 1; break;
    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
    default: break;
    }
  } while (i!=-1);

  /* register with cmdctrl */
  cc_init_options(argc,argv,0,0,0,0,QUIT_ON_QUIT);

  set_response(NLRSP_QUIET);	/* for norts Col_set_pointer */

  if ((Col_set_pointer(IDLER_COL_PTR_ID,&usage,0)))
    if (quit_no_dg)
      msg(MSG_EXIT_ABNORM,"Can't cooperate with DG");
    else
      errno = 0;
  else msg(MSG,"achieved cooperation with DG");

  if ( (qnx_osinfo(0, &osdata)) == -1)
    msg(MSG_WARN,"Can't get Operating System Info");
  else {
    msg(MSG,"Node Number: %ld",osdata.nodename);
    msg(MSG,"Processor: %ld; Floating Point Unit: %ld",osdata.cpu,osdata.fpu);
    msg(MSG,"CPU Speed: %u",osdata.cpu_speed);
    msg(MSG,"Tick Size: %u usecs",osdata.tick_size);
    msg(MSG,"QNX Version: %f; Release: %c",osdata.version/100.0,osdata.release);
    msg(MSG,"Total Memory: %uK",osdata.totmemk);
  }
  if ( (fd = open("./",O_RDONLY)) == -1)
    msg(MSG_WARN,"Can't open directory %s",getcwd(NULL,0));

  l = 0;
  while (!terminated) {
    if (l && !(l%LOOPER)) {
      elapse = (float)times(&cputim) / (float)CLK_TCK;
      usage[0] = 100 - ((((cputim.tms_stime/(float)CLK_TCK) +
		       (cputim.tms_utime/(float)CLK_TCK))/elapse)*100.0);
      msg(MSG_DEBUG,"CPU Usage: %d%%", usage[0]);
      qnx_osinfo(0, &osdata);
      usage[1] = 100-(((float)osdata.freememk/(float)osdata.totmemk) * 100.0);
      if (usage[1] > hi_mem) hi_mem = usage[1];
      msg(MSG_DEBUG,"Memory Usage: %d%%", usage[1]);
      if (fd)
	if (disk_space(fd, &free_b, &tot_b) == -1)
	  msg(MSG_DEBUG,"Can't get Disk Space Info");
	else {
	  usage[2] = 100-(((float)free_b/(float)tot_b)*100.0);
	  msg(MSG_DEBUG,"Disk Usage: %d%%", usage[2]);
	}
    }
    l++;
  }				/* while */

  setprio(0, oldpri);
    
  /* summary statistics */
  elapse = (float)times(&cputim) / (float)CLK_TCK;
  usage[0] = 100 - ((((cputim.tms_stime/(float)CLK_TCK) +
		   (cputim.tms_utime/(float)CLK_TCK))/elapse)*100.0);
  msg(usage[0] > 90 ? MSG_WARN : MSG,"Average Use of CPU: %d%%", usage[0]);
  msg(MSG,"Highest Recorded Memory Usage: %d%%",hi_mem);
  msg(MSG,"Disk Usage: %d%%", usage[2]);
  DONE_MSG;

}				/* main */
