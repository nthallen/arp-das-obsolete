#ifndef _TIMING_H_INCLUDED
#define _TIMING_H_INCLUDED

#include <dbr_utils.h>

#define MFC_TIME(TIMESTAMP,MFC) (TIMESTAMP.secs + ((MFC-TIMESTAMP.mfc_num) * dbr_info.nrowminf * tmi(nsecsper)) / tmi(nrowsper))

extern int findmf( time_t secs, int numfiles, char *dirname,
  char *rootname, int fperd, long *pos, long *nextstamp, tstamp_type *stamp,
   unsigned int *mfcounter);
extern int getnexttimestamp( int fd, tstamp_type *tstamp);
extern int gettimestamp( char *filename, int which, tstamp_type *stamp );
extern time_t gettimeval( char *string, time_t usetime );

#endif
