/*
	file time utilities.
	Written Aug 1991 by Eil.
	Modified and ported to QNX 4 by Eil 5/12/92.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <dbr.h>
#include <eillib.h>
#include <dbr_mod.h>
#include <rollover.h>

/*
   finds the next timestamp.
   call this routine first with a valid fd to give first stamp,
   thereafter with a NULL fd.
   returns positive if next time stamp found.
   returns (LAST_STAMP) if last timestamp.
   returns -2 if error or EOF reached
*/   
int getnexttimestamp( int fd, tstamp_type *tstamp ) {
static short s;
static int fd2;
static long length;
long jump;

    if (fd) {
	if (lseek(fd,0L,SEEK_SET) == -1)
	    return(-2);
	fd2 = fd;
	length = filelength(fd);
	if ( read( fd, tstamp, sizeof(tstamp_type)) < 1)
	    return(-2);
	if ( read( fd, &s, 2 ) < 1)
	    return(-2);
    }  else  {
	/* seek in file */
	if (!dbr_info.tm.nbrow) return (-2);
	if (s==LAST_STAMP) return(-2);
	jump = s * dbr_info.tm.nbrow;
	if ( ( (tell(fd2) + jump) > length) || (lseek( fd2, jump, SEEK_CUR) == -1) )
	    return(-2);
	if (read( fd2, tstamp, sizeof(tstamp_type)) < 1);
	    return(-2);
	if (read( fd2, &s, 2 ) < -1)
	    return(-2);
    }
    return(s);
}


/* 
 Retrieves a timestamp from a log file, 0 means the last timestamp, (given).
 Returns the number of the closest found timestamp, or -1 on error.
*/
int gettimestamp( char *filename, int which, tstamp_type *tstamp) {

int fd, fdarg;
int w, i;
short s;
	
	/* open file */
	tstamp->secs = 0; tstamp->mfc_num = 0;	
	if ( (fd = open( filename,O_RDONLY,0 )) == -1) return(-1);
	if ( which<=0 ) w = INT_MAX;
	else w = which;
	i = s = 0;
	fdarg = fd;
	while ( i<w ) {
		s = getnexttimestamp( fdarg, tstamp );
		if (s==-2 || s==LAST_STAMP) break;
		fdarg = 0;
		i++;
	}
	close(fd);
	if (s == -2) return(-1);
	if (s == LAST_STAMP) i++;
	return(i);
}


/* get a time value in secs for a string representation. usetime is a time
   value for substituting date components when there are dropouts in the
   string. String is of the form "MM/DD/YY/HH:MM:SS".
*/

time_t gettimeval( char *string, time_t usetime ) {
	
struct tm *t;
struct tm restime;
char *p1, *p2, *p3;
int mdy[3] = {-1,-1,-1};
int hms[3] = {-1,-1,-1};
char sep[3];
int i;
	
	/* get breakdown time for usetime */
	t = localtime( &usetime );
	restime = *t;
	/* parse string */
	p1 = strdup(string);
	strcpy(sep," /");
	p3=p1;
	if (strpbrk(p1,sep)) {
	    p2=strtok(p3,sep);
	    i=0;
	    p3=NULL;
	    while (p2) {
		mdy[i++] = atoi(p2);
		p2=strtok(p3,sep);
		if (i==2) break;
	    }
	}
	i=0;
	strcpy(sep,":");
	while ( (p2 = strtok(p3, sep)) !=NULL) {
		p3=NULL;
		hms[i++] = atoi(p2);
		if (i==3) break;
	}

	if (mdy[0]>-1) mdy[0]--;
	if (mdy[0]>-1) restime.tm_mon = mdy[0];
	if (mdy[1]>-1) restime.tm_mday = mdy[1];
	if (mdy[2]>-1) restime.tm_year = mdy[2];
	if (hms[0]>-1) restime.tm_hour = hms[0];
	if (hms[1]>-1) restime.tm_min = hms[1];
	if (hms[2]>-1) restime.tm_sec = hms[2];

	free(p1);
	return( mktime( &restime ) );
}

/*	
Find first occurence of appropiate mf given time.
return file number, file position of mf and mfc.
return -1 on error.
*/
int findmf( time_t secs, int startfile, int endfile, char *dirname, char *rootname,
	   int fperd, long *pos, long *nextstamp, tstamp_type *stamp,
		unsigned short *mfcounter ) {
	
int start, end, mid;
int fd;
tstamp_type stampvar;
tstamp_type thestamp;
long thenextstamp;
long position;
time_t filetime;
int i, j;
char buffer[FILENAME_MAX];
char *databuf;
unsigned short tmpmfc, mfc, refmfc;
int midtimes;

	/* initialisation */
	databuf = (char *)malloc(dbr_info.tm.nbminf);		
	fd = -1;

	/* binary search through file's header timestamps, first minor frame */
	start = startfile;  end = endfile;
	mid = -1; j = -1;
	midtimes = 0;

	do {
	    /* find mid file */
	    if (midtimes) {
		if (midtimes==1) j=mid;
		if (end==mid) {
		    end = (j==start) ? start : j-1;
		    midtimes=0;
		    j = -1;
		}
		mid++;
	    }

	    if (!midtimes)
		mid = start + (end - start) / 2;

	    /*  mid file */


	    if (mid == start) {
		midtimes++;
		if (start != end) continue;
	    }
	    else midtimes++;

	    if (fd != -1) {
		close(fd);
		fd = -1;
	    }
	    if ( (fd = open(getfilename( buffer, dirname, rootname, mid, fperd, 0),O_RDONLY, 0)) == -1) {
		msg(MSG_WARN,"Can't open Log File %s",buffer);
		continue;
	    }
	    /* read header timestamps until data */
	    i = 1;
	    do i = getnexttimestamp( i ? fd : 0, &stampvar); while (i==0);
	    if (i==-2) {
		msg(MSG_WARN,"Log File %d: Can't read head timestamps : file skipped",mid);
		continue;
	    }
	    position = tell(fd);
	    /* read first minor frame */
	    if (read(fd, databuf, dbr_info.tm.nbminf) != dbr_info.tm.nbminf) {
		msg(MSG_WARN,"Log File %d: Can't read 1st minor frame: file skipped",mid);
		continue;
	    }
	    /* calculate starting time of file */
	    tmpmfc = getmfc(databuf);
	    filetime = MFC_TIME(stampvar,tmpmfc);
	    /* reset limits */
	    if (filetime >= secs) {
		end = mid;
		if (j != -1) continue;
	    }
	    else start = mid;
	    midtimes = 0;
	    j = -1;

	}  while (start!=end);

	if (midtimes) {
	    if (startfile == endfile) return(-1);
	    msg(MSG_EXIT_ABNORM,"No Useable Log Files");
	}

	/* search forward in found file for further appropriate timestamps */
	thestamp = stampvar;
	thenextstamp = (i==LAST_STAMP) ? 0 : position + (i * tmi(nbrow));
	if (i!=LAST_STAMP) {
	    lseek(fd, position, SEEK_SET);
	    do
		do {
		    thestamp = stampvar;
		    j = i;
		    position = tell(fd);
		    i=getnexttimestamp(0,&stampvar);
		} while (i==0);
	    while ( i!=2 && i!=LAST_STAMP && stampvar.secs<=secs);
	    if (i==-2) {
		msg(MSG_WARN,"Log File %d: Can't read a timestamp",mid);
		return(-1);
	    }
	    lseek(fd, position, SEEK_SET);
	    thenextstamp = (j==LAST_STAMP) ? 0 : position + (j * tmi(nbrow));
	}

	/* calculate mfc number of desired time given by secs */
	if (secs==LONG_MAX)
	    mfc = tmpmfc + USHRT_MAX;
	else {
	    mfc = thestamp.mfc_num + ((secs - thestamp.secs) * tmi(nrowsper))/(tmi(nsecsper)*dbr_info.nrowminf);
	    if (LT(mfc,tmpmfc,tmpmfc)) mfc = tmpmfc;
	}

	/* search forward for minor frame until timestamp encountered or EOF */
	for (i=0, refmfc=tmpmfc; LT(tmpmfc,mfc,refmfc); i++) {
		position = tell(fd);
		j = read(fd, databuf, dbr_info.tm.nbminf);	    
		/* error detected */
		if (j == -1) {
		    msg(MSG_WARN,"Log File %d: Error while reading",mid);
		    return(-1);
		}
		/* end of file */		
		if (!j) {
		    if (i) position -= dbr_info.tm.nbminf;
		    else return(-1);
		    break;
		}
		/* break if not a complete mf */
		if (j != dbr_info.tm.nbminf) {
		    msg(MSG_WARN,"Log File %d: Incomplete minor frame at end",mid);
		    if (i) position -= (dbr_info.tm.nbminf + j);
		    else return(-1);
		    break;
		}
		/* check for timestamp position */
		if (tell(fd) == thenextstamp) break;
		tmpmfc = getmfc(databuf);
	}

    /* return filenumber and cleanup */
    if (stamp) *stamp=thestamp;
    if (mfcounter) *mfcounter=tmpmfc;
    if (nextstamp) *nextstamp=thenextstamp;
    if (pos) *pos=position;
    if (fd != -1) close(fd);
    free(databuf);
    return(mid);
}
