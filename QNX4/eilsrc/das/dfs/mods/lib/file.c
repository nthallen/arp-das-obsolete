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
   returns -2 if error.
*/   
int getnexttimestamp( int fd, tstamp_type *tstamp ) {
static short s;
static int fd2;
static long length;
long jump;

    if (fd) {
	lseek(fd,0L,SEEK_SET);
	fd2 = fd;
	length = filelength(fd);
	read( fd, tstamp, sizeof(tstamp_type) );
	read( fd, &s, 2 );		
    }  else  {
	/* seek in file */
	if (!dbr_info.tm.nbrow) return (-2);
	if (s==LAST_STAMP) return(-2);
	jump = s * dbr_info.tm.nbrow;
	if ( ( (tell(fd2) + jump) > length) || (lseek( fd2, jump, SEEK_CUR) == -1) )
		return(-2);
	read( fd2, tstamp, sizeof(tstamp_type) );
	read( fd2, &s, 2 );		
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
		unsigned int *mfcounter ) {
	
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

	/* initialisation */
	databuf = (char *)malloc(dbr_info.tm.nbminf);		
	fd = 0;

	/* binary search through file's header timestamps, first minor frame */
	start = startfile;  end = endfile;  mid = -1;
	do {
		/* find mid file */
		if (start == mid && end == (start+1))
		    mid = start = end;
		else if (end == mid && start == (end-1))
		    mid = end = start;
		else mid = start + (end - start) / 2;
		/*  mid file */
		if (fd) close(fd);
		if (!getfilename( buffer, dirname, rootname, mid, fperd, 0))
		    return(-1);
		/* Heres where we deal with gaps in log files, if desired */
		if ( (fd = open(buffer,O_RDONLY, 0)) == -1)
		    return(-1);
		/* read header timestamps until data */
		do i = getnexttimestamp( fd, &stampvar); while (i==0);
		if (i==-2) return(-1);
		position = tell(fd);
		/* read first minor frame */
		if (read(fd, databuf, dbr_info.tm.nbminf) != dbr_info.tm.nbminf)
		    return(-1);
		/* calculate starting time of file */
		tmpmfc = getmfc(databuf);
		filetime = MFC_TIME(stampvar,tmpmfc);
		/* reset limits */
		if (filetime >= secs) end = mid;
		else start = mid;
	} while (start!=end || start!=mid);


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
	    while (i!=LAST_STAMP && stampvar.secs<=secs);
	    if (i==-2) return(-1);
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
	for (refmfc=tmpmfc; LT(tmpmfc,mfc,refmfc); ) {
		position = tell(fd);
		j = read(fd, databuf, dbr_info.tm.nbminf);	    
		/* error detected */
		if (j == -1) return(-1);		
		/* end of file */		
		if (!j) {
		    position -= dbr_info.tm.nbminf;
		    break;
		}
		/* break if not a complete mf */
		if (j != dbr_info.tm.nbminf) {
		    position -= (dbr_info.tm.nbminf + j);
		    break;
		    /* return(-1); */
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
    close(fd);
    free(databuf);
    return(mid);
}
