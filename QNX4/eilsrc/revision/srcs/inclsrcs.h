#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#ifdef QNX
#include <qnx.h>
#include <sys/stat.h>
#include <process.h>
#include "lat.h"
#undef FMSIZE
#define FMSIZE 80
#include <io.h>
#include <lfsys.h>
#include <fileio.h>
#include <magic.h>
#include <time.h>
#include <signal.h>
#define poserr perror
#define stcpma(a,b) ((strstr(a,b)==a) ? strlen(b) :0)
#define stcpm(a,b,c) (*c=strstr(a,b))
#define _OSERR errno
#else
#include <dos.h>
#endif

/* token codes */
#define QUIT 0
#define REGISTIR 1
#define UPDATE 2
#define DELETE 3
#define PROJECT 4
#define HELP 5
#define WRITER 6
#define VERSION 7
#define LIST 8
#define COPY 9

#define LF 10
#define CR 13
#define RS 30
#define SEPARATORS ", "
#define LINE 80
#define NUMLINES 22
#define BUFSIZE 2000
#define NPTRS 130
#define NULS '\000'
#ifndef NULL
#define NULL 0
#endif

/* error codes */
#define DOSERROR 0
#define NOTFOUND 1
#define NOTDONE 2
#define BUFOVERFLOW 3
#define LISTERROR 4
#define MATCHERROR 5
#define NOPROJECT 6
#define NOSYS 7
#define READERROR 8
#define BADCOMMAND 9

#define CAN_READ(filename) (access(filename,4)==0)
#define CAN_WRITE(filename) (access(filename,2)==0)
#define EXISTS(filename) (access(filename,0)==0)


