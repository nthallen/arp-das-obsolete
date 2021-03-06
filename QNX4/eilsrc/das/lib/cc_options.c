/*
    Command Control registration for DAS programs -- options.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <process.h>
#include "cmdctrl.h"
#include "msg.h"
#include "cc.h"

extern char *opt_string;

int cc_init_options(int argcc, char *argvv[], unsigned char min_dasc,
		    unsigned char max_dasc, unsigned char min_msg,
		    unsigned char max_msg, quit_type how_to_quit, ...) {
  extern char *optarg;
  extern int optind, opterr, optopt;
  int c;
  int action=NOTHING_ON_DEATH;
  char cmd_line[MAX_MSG_SIZE];
  char ts[MAX_MSG_SIZE];
  int proc_wd_only = 0;
  va_list args;
  pid_t proxy=0;

  opterr = 0;
  optind = 0;

  do {
    c=getopt(argcc,argvv,opt_string);
    switch (c) {
    case 'O': proc_wd_only = 1; break;
    case 'R': action=REBOOT; break;
    case 'D': action=DAS_RESTART; break;
    case 'T': action=TASK_RESTART; break;
    case 'S': action=SHUTDOWN; break;
    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
      default : break;
    }
  }  while (c!=-1);

  opterr = 1;

  if (action==TASK_RESTART) {
    if (getcmd(cmd_line)==NULL)
      msg(MSG_EXIT_ABNORM,"Can't get command line args");
    sprintf(ts,"%s %s",argvv[0],cmd_line);
  }
  if (how_to_quit==PROXY_ON_QUIT) {
    va_start(args,how_to_quit);
    proxy=va_arg(args,pid_t);
    va_end(args);
  }

  if (proc_wd_only)
    return(cc_init(0, 0, 0, 0, NOTHING_ON_QUIT, action, 0, ts));
  else
    return(cc_init(min_dasc,max_dasc,min_msg,max_msg,how_to_quit,action,proxy,ts));
}
