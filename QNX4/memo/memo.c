/*
    Memo Program.
    Written by Eil Aug 1992.
*/

/* header files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/psinfo.h>
#include <sys/types.h>
#include "memo.h"
#include "reply.h"
#include "eillib.h"
#include "oui.h"

static int aud = 0;

void memo_init_options(int argc, char **argv) {
  /* getopt variables */
  extern char *optarg;
  extern int optind, opterr, optopt;
  nid_t n;
  int i;
  reply_type rv;
  /* process args */
  opterr = 0;
  optind = 0;
  do {
    i=getopt(argc,argv,opt_string);
    switch (i) {
    case 'q': aud = 1; break;
    case 'k':
      n = atoi(optarg);
      rv = MEMO_DEATH_HDR;
      if ( (who = qnx_name_locate(n,LOCAL_SYMNAME(MEMO),0,0))!=-1) {
	if (Send(who,&rv,&rv,sizeof(reply_type),sizeof(reply_type))==-1)
	  msg(MSG_EXIT_ABNORM,"error sending to %s: task %d",MEMO,who);
	if (rv!=REP_OK)
	  msg(MSG_EXIT_ABNORM,"bad response from %s: task %d",MEMO,who);
      }
      else msg(MSG_EXIT_ABNORM,"Can't find %s on node %d",MEMO,n);
      msg(MSG,"task %d: completed",getpid());
      msg_end();
      exit(0);
    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
    default: break;
    }
  } while (i!=-1);
}

main(int argc, char **argv) {
  char recv_msg[MEMO_MSG_MAX];
  pid_t who;
  int got_quit = 0;
  reply_type rv;

  /* initialise das options from command line */    
  oui_init_options(argc,argv);
  BEGIN_MSG;

  /* register yourself */
  if ((qnx_name_attach(getnid(),LOCAL_SYMNAME(MEMO)))==-1)
    msg(MSG_EXIT_ABNORM,"Can't attach symbolic name for %s",MEMO);

  /* sort messages by priority */
  qnx_pflags(~0,_PPF_PRIORITY_REC,0,0);

  while(1) {
    rv = REP_OK;
    errno=0;

    if (!got_quit)
      while ((who=Receive(0,recv_msg,MEMO_MSG_MAX))==-1)
	msg(MSG_WARN,"error on receive");	
    else
      if ((who=Creceive(0,recv_msg,MEMO_MSG_MAX))==-1) {
	msg(MSG,"message queue empty and quit received");
	msg(MSG,"task %d: completed",getpid());
	msg_end();
	exit(0);
      }

    recv_msg[MEMO_MSG_MAX-1] = '\000';
    switch (recv_msg[0]) {
    case MEMO_HDR:
      if (Reply(who, &rv, sizeof(reply_type))==-1)
	msg(MSG_WARN,"error replying to task %d",who);
      i=MSG;
      if (aud==1)
	if (strstr(recv_msg,FATAL_STR) || strstr(recv_msg,FAIL_STR))
	  i=MSG_FAIL;
	else if (strstr(recv_msg,WARN_STR)) 
	  i=MSG_WARN;
	else if (strstr(recv_msg,DEBUG_STR)) 
	  i=MSG_DEBUG;
      msg(i,"%s",recv_msg+1);
      break;
    case MEMO_DEATH_HDR:
      if (Reply(who, &rv, sizeof(reply_type))==-1)
	msg(MSG_WARN,"error replying to task %d",who);
      got_quit = 1;
      break;
    default:
      rv = REP_UNKN;
      msg(MSG_WARN,"unrecognised msg received");
      if (Reply(who, &rv, sizeof(reply_type))==-1)
	msg(MSG_WARN,"error replying UNKNOWN to task %d",who);
    }
  }				/* while */
}
