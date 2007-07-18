/*
	msg.c: an informing system.
	written by Eil.
	ported to QNX 4 by Eil 4/2/92.
*/  

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <process.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef __QNX__
#include <sys/dev.h>
#include <sys/qnxterm.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/proxy.h>
#endif
#include "reply.h"
#include "memo.h"
#include "symname.h"
#include "msg.h"
#include "sounds.h"

/* IBM compatable display attributes for console functions */
#define IBM_NORMAL 0x07
#define IBM_REVERSE 0x70
#define IBM_BLINK (IBM_NORMAL | 0x80)
#define IBM_BOLD 0x0F

#define LEN 200

/* Vars are default initialised in case msg_init() dosnt get called */
static int msg_inited;
static int msg_verbose = 1;
static int msg_fd = -1;
#ifdef __QNX__
static struct _console_ctrl *msg_cc = NULL;
static int scrwidth = 0;
static pid_t memo_tid;
#endif
static unsigned int msg_pass = 0;
static unsigned int msg_warn = 0;
static unsigned int msg_fail = 0;
static unsigned int msg_debug = 0;
static int msg_row = 0;
static int msg_col = 0;
static int msg_size = 0;
static int msg_devfd = -1;
static char msg_hdr[40] = {'\0'};
static int msg_sounds = 1;
static int msg_sys = 1;
static int msg_level = 0;
static char *msg_buf = 0;
static char *msg_fbuf = 0;
static char *msg_dbuf = 0;
static time_t timevar;

/* returns true if messages already initialised */
int msg_initialised(void) {
  if (msg_inited) return 1;
  return 0; 
}

/* ends messages */
void msg_end() {
  if (msg_fd != -1) close(msg_fd);
#ifdef __QNX__
  if (msg_devfd != -1) close(msg_devfd);
  if (msg_cc) console_close(msg_cc);
#endif
  if (msg_buf) free(msg_buf);
  if (msg_fbuf) free(msg_fbuf);
  if (msg_dbuf) free(msg_dbuf);
  msg_inited=0;
}

/*  write an informative message, notes in msg.h */
void msg(int fatal, char *format,...) {
  static int exitflag;
  va_list ap;
  char errline[LEN] = {MEMO_HDR,'\0'};
  char *p;
  reply_type replymsg;
  int msgsize, i;
#ifdef __QNX__
  unsigned attr;
  pid_t prxy;
#endif

  /* form error string */	
  if (!format && (!errno || !msg_sys) && fatal!=MSG_EXIT_ABNORM && 
      fatal!=MSG_EXIT_NORM)
    return;

  /* debug level checking */
  if (fatal <= MSG_DEBUG && fatal < MSG_DEBUG-msg_level+1) return;

  va_start(ap, format);
  if (msg_hdr)
    if (strlen(msg_hdr)) {
      strncat(errline, msg_hdr, 10);
      strcat(errline, ": ");
    }

  switch(fatal) {
  case MSG_PASS: strcat(errline,PASS_STR); break;
  case MSG_EXIT_NORM: strcat(errline,PASS_STR); break;
  case MSG_DEBUG: strcat(errline,DEBUG_STR); break;
  case MSG_WARN: strcat(errline,WARN_STR); break;
  case MSG_FAIL: strcat(errline,FAIL_STR); break;
  default: if (fatal < MSG_DEBUG) { strcat(errline,DEBUG_STR); break; }
  case MSG_EXIT_ABNORM: strcat(errline,FATAL_STR); break;
  }

  p = errline + strlen(errline);
  vsprintf(p, format, ap);
  p = errline + 1;
  if ( errno && msg_sys )  {
    strcat(p,": ");
    strcat(p,strerror(errno)); 
  }
  va_end(ap);
  msgsize = strlen(p);

  /* log to MEMO */
#ifdef __QNX__
  if (memo_tid != getpid() ) {
    /* Send blocks, would be better if we didnt block here, but attaching
       a proxy to someone else means that someone else has to detach it
    */
    /*prxy=qnx_proxy_attach(memo_tid,errline,(msgsize+2>100)?100:msgsize+2,-1);
	Trigger(prxy);
    */
   Send(memo_tid, errline, &replymsg, msgsize+2, sizeof(reply_type));
  }
#endif

  /* log to file */		
  if (msg_fd != -1)  {
    time(&timevar);
    strftime(msg_dbuf,10,"%T",localtime(&timevar));
    sprintf(msg_fbuf,"%s: %s\n",msg_dbuf,p);
    write(msg_fd,msg_fbuf,strlen(msg_fbuf));
  }

  /* to stderr */
  if (msg_verbose)  {
    fprintf(stderr,"%s\n",p);
    fflush(stderr);
  }

  if (msg_devfd != -1) {
    switch (fatal) {
    case MSG_WARN: attr = msg_warn; break; /* warning */
    case MSG_DEBUG: attr = msg_debug; break; /* debug */
    case MSG_PASS: attr = msg_pass; break; /* just a msg */
    case MSG_FAIL: attr = msg_fail; break; /* error, no exit */
    case MSG_EXIT_NORM: attr = msg_pass; break; /* exit normally */
    default: if (fatal < MSG_DEBUG) { attr = msg_debug; break; }
    case MSG_EXIT_ABNORM: attr = msg_fail; /* exit abnormally */
    }

#ifdef __QNX__
    if (!msg_cc) {
      if (isatty(msg_devfd)) {
	for (i=0;i<msg_size;i++)
	  term_type(msg_row,msg_col+i," ",1,msg_pass);
	term_type(msg_row,msg_col,p,(msg_size<=strlen(p))?msg_size:0,attr);
	term_flush();
      } else {
	for (i=0;i<msg_size+msg_col+6;i++)
	  msg_buf[i]=' ';
	sprintf(msg_buf+msg_col,"\033S\033%c%s",attr,p);
	msg_buf[msg_col+4+strlen(p)]=' ';
	msg_buf[msg_size+msg_col+3]='\0';
	strcat(msg_buf,"\n\033R");
	write(msg_devfd,msg_buf,msg_col+msg_size+6);
      }
    } else {
      console_read(msg_cc,0,2*(msg_row*scrwidth+msg_col),msg_buf,
		   msg_size*2,0,0,0);
      for (i=0;i<msg_size*2;i++)
	msg_buf[i]=' ';
      for (i=0;i<(msg_size*2);i+=2,p++)
	*(msg_buf+i)=*p;
      for (i=1;i<=(msg_size*2);i+=2)
	*(msg_buf+i)=(char)attr;
      console_write(msg_cc,0,2*(msg_row*scrwidth+msg_col),msg_buf,
		    msg_size*2,0,0,0);
    }
  }
#endif
	
  errno=0;

  if (msg_sounds)
    switch (fatal) {
    case MSG_WARN: WARN_NOTE; break;
    case MSG_FAIL: FAIL_NOTE; break;
    case MSG_DEBUG:
    case MSG_PASS: break;
    case MSG_EXIT_NORM: break;
    default: if (fatal < MSG_DEBUG) break;
    case MSG_EXIT_ABNORM: FAIL_TUNE; break;
    }

  switch (fatal) {
  case MSG_EXIT_NORM: if (exitflag++) _exit(0);
    exit(0); break;
  default: if (fatal < MSG_EXIT_ABNORM) break;
  case MSG_EXIT_ABNORM: if (exitflag++) _exit(1);
    exit(1); break;
  }
}

/* initialise messages options */
void msg_init(char *hdr, char *fn, int verbose, char *targ, char *oarg,
	      int sounds, int sys, int level) {
#ifdef __QNX__
  nid_t nd;
  char memo_name[NAME_MAX] = MEMO;
#endif
  char devfn[80] = {'\0'};
  int j;
  char *p;

  msg_verbose = verbose;
  msg_sounds = sounds;
  msg_sys = sys;
  if (hdr && strlen(hdr)) strncpy(msg_hdr,hdr,39);
  msg_level = level < 0 ? 0 : level;
  if (oarg && strlen(oarg)) {
    j = strcspn(oarg,",;");
    p = oarg+j+1;	
    oarg[j]='\0';
    strncat(devfn,oarg,79);
    if (!strlen(devfn)) msg_devfd = STDERR_FILENO;
    j = strcspn(p,",;"); p[j] = '\0';
    msg_row=atoi(p); p = p+j+1;
    j = strcspn(p,",;"); p[j] = '\0';
    msg_col=atoi(p); p = p+j+1;
    j = strcspn(p,",;"); p[j] = '\0';
    msg_size=atoi(p); p = p+j+1;
    j = strcspn(p,",;"); p[j] = '\0';
    msg_pass = (unsigned)atoh(p); p = p+j+1;
    j = strcspn(p,",;"); p[j] = '\0';
    msg_warn = (unsigned)atoh(p); p = p+j+1;
    j = strcspn(p,",;"); p[j] = '\0';
    msg_fail = (unsigned)atoh(p); p = p+j+1;
    j = strcspn(p,",;"); p[j] = '\0';
    msg_debug = (unsigned)atoh(p);
  }

#ifdef __QNX__
  memo_tid = getpid();
  if (targ && strlen(targ)) {
    j = strcspn(targ,",;");
    p = targ+j+1;	
    targ[j]='\0';
    nd = atoi(targ);
    j = strcspn(p,",;"); p[j] = '\0';
    if (strlen(p)) strncpy(memo_name,p,NAME_MAX-1);
    if ( (memo_tid=qnx_name_locate(nd, LOCAL_SYMNAME(memo_name),0,0))==-1)
      msg(MSG_EXIT_ABNORM,
	  "Can't find symbolic name for %s on node %ld",memo_name,nd);
  }
#endif

  if (fn && strlen(fn))
    if ((msg_fd=open(fn, O_WRONLY | O_CREAT | O_APPEND | O_DSYNC,
	    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1)
      msg(MSG_EXIT_ABNORM,"Can't open error file %s",fn);
    else {
      if (errno == ENOENT) errno = 0;
      msg_fbuf = malloc(LEN+10);
      msg_dbuf = malloc(10);
      time(&timevar);
      strftime(msg_dbuf,10,"%D",localtime(&timevar));
      sprintf(msg_fbuf,"Log Date: %s\n",msg_dbuf);
      write(msg_fd,msg_fbuf,strlen(msg_fbuf));
    }
	    
#ifdef __QNX__
  if ( devfn && strlen(devfn))
    if ((msg_devfd = open(devfn,O_RDWR | O_CREAT | O_APPEND | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) == -1)
      msg(MSG_EXIT_ABNORM,"Can't open %s",devfn);
  if ( (msg_cc = console_open(msg_devfd,O_RDWR))) {
    console_size(msg_cc,0,0,0,0,&scrwidth);
    msg_size = (msg_size <= 0) ? scrwidth-msg_col : msg_size;
    msg_buf=(char *)malloc(2*msg_size);
    msg_fail = msg_fail ? msg_fail : IBM_BLINK;
    msg_warn = msg_warn ? msg_warn : IBM_REVERSE;
    msg_pass = msg_pass ? msg_pass : IBM_NORMAL;
    msg_debug = msg_debug ? msg_debug : IBM_BOLD;
  } else {
    if (msg_devfd >=0 && isatty(msg_devfd)) {
      tcsetct(msg_devfd, getpid());
      if (term_load()) {
	close(msg_devfd);
	msg_devfd = -1;		    	
	msg(MSG_EXIT_ABNORM,"Can't load terminal definition");
      }
      scrwidth = term_state.num_cols;
      msg_size = (msg_size <= 0) ? scrwidth-msg_col : msg_size;
      msg_fail = msg_fail ? msg_fail : TERM_BLINK;
      msg_warn = msg_warn ? msg_warn : TERM_INVERSE;
      msg_pass = msg_pass ? msg_pass : TERM_NORMAL;
      msg_debug = msg_debug ? msg_debug : TERM_HILIGHT;
    } else {
      errno=0;
      scrwidth = 80;
      msg_size = (msg_size <= 0) ? scrwidth-msg_col : msg_size;
      msg_fail = msg_fail ? msg_fail : '{';
      msg_warn = msg_warn ? msg_warn : '(';
      msg_pass = msg_pass ? msg_pass : 'S';
      msg_debug = msg_debug ? msg_debug : '<';
      msg_buf=(char *)malloc(msg_size+msg_col+10);
    }
  }
#endif
  msg_inited = 1;
}
