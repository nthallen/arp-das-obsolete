/*
    DBR Reader data handler.
    Written Aug 1991 by Eil.
*/
	
/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <i86.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>
#include <eillib.h>
#include <das.h>
#include <dbr.h>
#include <dbr_mod.h>
#include <globmsg.h>

#define CLOSEFILE { \
    close(curfd); \
    curfd=0; \
}

/* global variables */
extern char rootname[];
extern char dirname[];
extern int filesperdir;
extern int endfile;
extern int startfile;
extern time_t starttime;	/* start time of logged files */
extern time_t endtime;		/* end time of logged files */
extern char *rowbuffer;
extern char *mfbuffer;
extern int setfromfile;
extern int settofile;
extern long setfrompos, settopos;
extern long setnexttimepos;
extern tstamp_type set_stamp;
extern unsigned int sleeper;
extern int quitter;

static int fromfile;
static int tofile = 0;
static long frompos;
static long topos = 0;
static long nexttimepos;
static char curfilename[FILENAME_MAX+1];
static long curposition = 1;
static int curfd = 0;
static int curfile = 1;


int DG_get_data( token_type n_rows ) {

tstamp_type T;
int mfcount, mfs;
static int rowcount;
int i, j;
short next_ts;
char byt1, byt2;

assert(n_rows);

if (sleeper) delay(sleeper);

/* figure number of mf's neaded to be read */
mfs = (n_rows - rowcount) / dbr_info.nrowminf;
if ( (n_rows - rowcount) % dbr_info.nrowminf ) mfs++;

for (mfcount = 0; mfcount < mfs; ) {

    /* 1) Startup and Endup */
    if ( curposition >= topos && curfile > tofile ) {
	if ( mfcount || rowcount ) break;
	if (curfd) {
	    CLOSEFILE;
	    if (!quitter) DG_s_dascmd(DCT_TM,DCV_TM_END);	    
	    else DG_s_dascmd(DCT_QUIT,DCV_QUIT);
	} else {
	    frompos=setfrompos;
	    fromfile=setfromfile;
	    topos=settopos + tmi(nbminf);
	    tofile=settofile;
	    curfile=setfromfile;
	    curposition=frompos;
	    nexttimepos=setnexttimepos;
	    dbr_info.t_stmp=set_stamp;
	    DG_s_tstamp(&dbr_info.t_stmp);
	}
	return 1;
    }

    /* 2) Start of File */
    if (curfd <= 0 && curfile <= tofile)
	if (getfilename( curfilename, dirname, rootname, curfile++, filesperdir, 0))
	    if ( (curfd = open( curfilename, O_RDONLY, 0))  != -1) {
		if (lseek(curfd, curposition, SEEK_SET) == -1) {
		    msg(MSG_EXIT_ABNORM,"Can't seek to position %d in %s",curposition,basename(curfilename));
		    nexttimepos=0;
		    CLOSEFILE;
		    continue;
		}
	    }
	    else {
		msg(MSG_WARN,"Can't open file %s",basename(curfilename));
		curfd=0;
		continue;
	    }
	else {
	    msg(MSG_WARN,"error creating filename %s",basename(curfilename));
	    continue;
	}

    curposition = tell(curfd);

    /* 3) Time Stamp */
    if (curposition == nexttimepos)
	if (mfcount) break;
	else {
	    /* read and send timestamp */
	    j = read( curfd, &T, sizeof(tstamp_type) );
	    i = read( curfd, &next_ts, 2 );
	    /* end of file, error or bad file, this shouldn't happen, error */
	    if (j != sizeof(tstamp_type) || i != 2) {
		if (j == -1 || i == -1)
		    msg(MSG_WARN,"Can't read timestamp from file %s: remainder of file skipped",basename(curfilename));
		else msg(MSG_WARN,"Incomplete timestamp at end of file %s",basename(curfilename));
		CLOSEFILE;
		curposition = 0;
		continue;
	    }  else  {
		curposition = tell(curfd);
		nexttimepos = (next_ts==LAST_STAMP) ? 0 : curposition + (next_ts * tmi(nbrow));
		if (T.secs != dbr_info.t_stmp.secs || T.mfc_num != dbr_info.t_stmp.mfc_num) {
		    dbr_info.t_stmp=T;
		    DG_s_tstamp(&T);
		    return 1;
		}
	    }
	}

    /* 4) Data */
    /* get a whole minor frame of data */
    else {
	j = read( curfd, mfbuffer+(mfcount*tmi(nbminf)), tmi(nbminf) );
	curposition = tell(curfd);
	if (j == tmi(nbminf) && check_synch(mfbuffer+mfcount*tmi(nbminf)))
	    mfcount++;
	else {
	    if (j != tmi(nbminf)) {
		if (j == -1) msg(MSG_WARN,"error reading file %s, remainder of file skipped",basename(curfilename));
		else if (j) msg(MSG_WARN,"Incomplete minor frame at end of file %s",basename(curfilename));
		if ( curposition < topos || curfile <= tofile) {
		    CLOSEFILE;
		    curposition = 0;
		}
	    }
	    else {
		/* synch problem */
		msg(MSG_WARN,"Lost synch in file %s",basename(curfilename));
		byt1 = ~(tmi(synch) & 0xFF);
		while ( (j=read(curfd, &byt2, 1)) == 1 )
		    if ( (byt1 == (tmi(synch) & 0xFF) && byt2 == (tmi(synch) >> 8)) ||
			(tmi(isflag) && byt1 == (tmi(synch) >> 8) && byt2 == tmi(synch) & 0x0F))
			break;
		    else byt1 = byt2;
		switch (j) {
		    case 0:  CLOSEFILE; curposition = 0;
		    case 1:  msg(MSG,"found synch"); break;
		    default:
		    case -1: msg(MSG_WARN,"error reading file %s, remainder of file skipped",basename(curfilename));
		}
	    }
	    continue;
	}
    }
}

/* send data */
j = (mfcount==mfs) ? n_rows - rowcount : mfcount * dbr_info.nrowminf;
if (rowcount) DG_s_data(rowcount, rowbuffer, j, mfbuffer);
else DG_s_data(j, mfbuffer, 0, 0 );
rowcount = mfcount * dbr_info.nrowminf - j;
memcpy(rowbuffer, mfbuffer + j * tmi(nbrow), rowcount * tmi(nbrow));

return 1;
}


int DG_other(unsigned char *msg_ptr, pid_t sent_tid) {
/* called when a message is recieved which is not handled by the distributor */
unsigned char r=DAS_OK;

    switch (*msg_ptr) {
	case TIME_SET:
	    switch( *(msg_ptr+1)) {
		case TIME_START:
		    set_time(msg_ptr+2,starttime,&setfromfile,&setfrompos,&setnexttimepos,&set_stamp,"Start");
		    break;
		case TIME_END:
		    set_time(msg_ptr+2,endtime,&settofile,&settopos,0,0,"End");
		    break;
		default: 
		    msg(MSG_WARN,"Unknown TIME_SET type %u",*(msg_ptr+1));
		    r=DAS_UNKN;
	    }
	    break;
	default: 
	    msg(MSG_WARN,"Unknown msg received of type %u",*msg_ptr);
	    r=DAS_UNKN;
    }
    if (Reply(sent_tid, &r, 1)==-1)
	msg(MSG_WARN,"error replying to task %d",sent_tid);
    return 1;
}


int DG_DASCmd( unsigned char type, unsigned char num ) {
/* called when DG recieves a DASCmd */	
return 1;
}

void set_time(char *m, time_t t, int *f, long *p,  long *n, tstamp_type *s, char *h) {
time_t timet, timet2;
unsigned short mf;

    timet=gettimeval( m, t);
    if (timet<starttime || timet>endtime) {
	msg(MSG_WARN,"Requested %s time: out of range: reset to Limit %s time",h,h);
	timet=t;
    }

    msg(MSG,"Requested %s time: %.24s %s",h,ctime(&timet),tzname[0]);
    /* find data start position */
    if ( (*f = findmf( timet, startfile, endfile, dirname, rootname, filesperdir, p, n, s, &mf)) == -1 )
	msg(MSG_EXIT_ABNORM,"%s time: Can't find appropriate minor frame",h);

    timet2 = MFC_TIME((*s),mf);
    if (timet2 != timet)
	msg(MSG,"Actual %s time: %.24s %s",h,ctime(&timet),tzname[0]);

}
