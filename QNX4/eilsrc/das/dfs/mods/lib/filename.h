#ifndef _FILE_H_INCLUDED
#define _FILE_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>

#define K 1024
#define ROOTLEN 8
#define FILESIZE (10*K)
#define MAXFILESIZE SHRT_MAX
#define FILESPERDIR 100
#define MAXFILESPERDIR SHRT_MAX
#define LAST_STAMP -1
#define ROOTNAME "log"
#define FILECOUNT_PROG "lfctr"

extern char *getfilename(char *, char *, char *, int, int, int);

#ifdef __cplusplus
};
#endif

#endif
