/*
    these are the fatal codes for the function 'msg'.
    msg is displayed at (row,col), in an area of 'size' with attribute,
    sound and prepended string according to the fatal code and options:
    MSG_PASS: pass attribute., PASS_STR.
    MSG_DEBUG : debug attribute, DEBUG_STR.
    MSG_WARN: warn attribute, WARN_NOTE and WARN_STR.
    MSG_FAIL: fail attribute, FAIL_NOTE and FAIL_STR;
    MSG_EXIT: pass attribute, normal exit.
    MSG_FATAL: fail attribute, FAIL_TUNE, FATAL_STR, exit with error status.
 $Log$
*/

#ifndef _MSG_H_INCLUDED
#define _MSG_H_INCLUDED

#include <sys/types.h>
#include <unistd.h>

/* standard action codes for msg() */
/* codes above MSG_FATAL are treated as MSG_EXIT_ABNORM */
/* codes below MSG_DEBUG are debug sub levels */
/* the order of these codes is important */
#define MSG_DEBUG -2
#define MSG_EXIT -1
#define MSG_EXIT_NORM MSG_EXIT
#define MSG 0
#define MSG_PASS MSG
#define MSG_WARN 1
#define MSG_FAIL 2
#define MSG_FATAL 3
#define MSG_EXIT_ABNORM MSG_FATAL

#define MSG_DBG(X) (MSG_DEBUG-(X))

/* strings that are prepended to message based on fatal code */
#define FATAL_STR "FATAL: "
#define WARN_STR "WARNING: "
#define FAIL_STR "ERROR: "
#define DEBUG_STR "DEBUG: "
#define PASS_STR ""

/* standard messages for msg() */
#define MSG_BEGIN "task started"
#define MSG_DONE "task completed"

#define BEGIN_MSG msg(MSG,"task %d: started",getpid())
#define DONE_MSG msg(MSG_EXIT_NORM, "task %d: completed",getpid())

/* initialises message options */
extern void msg_init(char *hdr, char *errfile, int verbose,
		     char *task_output, char *device_output,
		     int sounds_flag, int sys_error_flag, int level);
/*
    initialises message options from command line arguments.
    they are:
	-e <errorfile>
	-h <header>
	-c <node>[,<name>]
	-v
	-s
	-y
	-o <devicename[,row,col,size,pass_attribute,warn_attribute,fail_attribute>]
	-l 
*/
void msg_init_options(char *default_hdr, int argcc, char *argvv[]);
/* ends messaging */
extern void msg_end(void);
/* returns true if messages already initialised */
extern int msg_initialised(void);
/* write a msg, with action, sound and attribute according to 'fatal' */
extern void msg(int fatal, char *format,...);
/* global option string. Probably defined by programmer. */

#ifdef __QNX__
#define OPT_MSG_INIT "e:h:o:c:lvsy"
#else
#define OPT_MSG_INIT "e:h:o:lvsy"
#endif

#endif
