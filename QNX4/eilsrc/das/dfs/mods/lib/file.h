#ifndef _TIMING_H_INCLUDED
#define _TIMING_H_INCLUDED

#include <dbr.h>

extern int findmf( time_t secs, int startfile, int endfile, char *dirname,
  char *rootname, int fperd, long *pos, long *nextstamp, tstamp_type *stamp,
   unsigned short *mfcounter);
extern int getnexttimestamp( int fd, tstamp_type *tstamp);
extern int gettimestamp( char *filename, int which, tstamp_type *stamp );
extern time_t gettimeval( char *string, time_t usetime );
extern int check_synch(unsigned char *databuf);
extern unsigned int getmfc(unsigned char *databuf);

#endif
