/*
    Logger client functions.
    Assumes mf's are whole and guarenteed.
    Assumes the DG always sends appropriate timestamps.
    Written July 1991 by Eil.
    Modified for QNX 4 4/30/92 by Eil.
*/

/* includes */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include "globmsg.h"
#include "eillib.h"
#include "das.h"
#include "dbr.h"
#include "dbr_mod.h"

/* defines */
#define CLOSEFILE { \
    logged_bytes=0; \
    if (fp!=NULL) fclose(fp); \
    fp=NULL; \
}

/* fopen sets errno (=2) if the file dosn't already exist */
#define SWITCHFILE { \
    CLOSEFILE; \
    if ( (fp=fopen(getfilename(name,dirname,rootname,fcount++,filesperdir,1),"w"))==NULL ) \
	msg(MSG_EXIT_ABNORM,"Can't open file %s",basename(name)); \
    if (errno == ENOENT) errno = 0; \
}

/* global variables */
extern int logging;
extern int maxfilesize;
extern int fcount;
extern char rootname[ROOTLEN];
extern char dirname[FILENAME_MAX];
extern int filesperdir;
extern char name[FILENAME_MAX];

static FILE *fp;
static long logged_bytes;   /* number of bytes logged to current memory buffer */
static short rows_since_stamp;	/* number of rows logged since last timestamp */
static int minfrow;

void DC_data(dbr_data_type *dr_data) {
int bytes_to_log;
int rows_to_log;
int rows_space;
int rows_left_in_mf;
dbr_data_type *remaining_data;
tstamp_type t;

    if (logging && dr_data->n_rows) {

	/* available space to fit rows on mf boundary, in rows */
	rows_space = 0;
	rows_left_in_mf = dbr_info.nrowminf - minfrow;
	if (rows_left_in_mf==dbr_info.nrowminf) rows_left_in_mf=0;
	if (logged_bytes < maxfilesize)
	    rows_space = ((maxfilesize-logged_bytes-rows_left_in_mf*tmi(nbrow))/(dbr_info.nrowminf*tmi(nbrow)))*dbr_info.nrowminf + rows_left_in_mf;

	/* rows that fit */
	rows_to_log = min(rows_space, dr_data->n_rows);

	if (!rows_to_log || !fp)  {
	    SWITCHFILE;
	    rows_to_log = dr_data->n_rows;
	}
		
	bytes_to_log = rows_to_log*tmi(nbrow);
		
	/* initial time stamp for all files */
	if (!logged_bytes) DC_tstamp(&dbr_info.t_stmp);
		
	/* write data to file */
	if (fwrite(dr_data->data, 1, bytes_to_log, fp)!=bytes_to_log)
	    msg(MSG_EXIT_ABNORM,"error writing %d bytes to %s",bytes_to_log,basename(name));
		
	/* update number of bytes written */
	logged_bytes += bytes_to_log;
	minfrow=(minfrow+rows_to_log)%dbr_info.nrowminf;
	rows_since_stamp += rows_to_log;
		
	/* if there is any remaining data, log that too */
	if (dr_data->n_rows != rows_to_log) {
	    if ( !(remaining_data=malloc(2+(dr_data->n_rows-rows_to_log)*tmi(nbrow))))
		msg(MSG_EXIT_ABNORM,"Can't allocate memory");
	    remaining_data->n_rows = dr_data->n_rows - rows_to_log;
	    memmove(remaining_data->data, dr_data->data+bytes_to_log, (dr_data->n_rows - rows_to_log)*tmi(nbrow));
	    DC_data( remaining_data );
	    free( remaining_data );
	}
    }
}


void DC_tstamp( tstamp_type *tstamp )  {
/* size of the next stamp locator is 2 bytes */
short ls = LAST_STAMP;

    if (!minfrow) {
	
	if (logging)  {

	    /* check file has room to log timestamp plus trailing int */
	    if (!fp || logged_bytes + sizeof(tstamp_type) + 2 > maxfilesize)
		SWITCHFILE;

	    /* write last timestamp's trailing bytes indicating number of rows between the timestamps */
	    if (logged_bytes) {
		if ( fseek(fp,ftell(fp)-(rows_since_stamp * tmi(nbrow) + 2), SEEK_SET)!=0)
		    msg(MSG_EXIT_ABNORM,"Can't seek in %s",basename(name));
		if (fwrite( &rows_since_stamp, 2, 1, fp)!=1)
		    msg(MSG_EXIT_ABNORM,"Can't write next tstamp indicator in %s",basename(name));
		if (fseek(fp,0L,SEEK_END)!=0)
		    msg(MSG_EXIT_ABNORM,"Can't seek in %s",basename(name));
	    }
			
	    /* write time stamp to file */		
	    if (fwrite(tstamp, sizeof(tstamp_type), 1, fp)!=1)
		msg(MSG_EXIT_ABNORM,"Can't write tstamp in %s",basename(name));

	    /* write trailing LAST_STAMP for now */
	    if (fwrite(&ls, 2, 1, fp)!=1)
		msg(MSG_EXIT_ABNORM,"Can't write tstamp follower in %s",basename(name));
		
	    /* keep track of how many bytes logged to current memory buffer */		
	    logged_bytes += ( sizeof( tstamp_type ) + 2 );
		
	    /* reset rows since stamp */
	    rows_since_stamp = 0;
	}
    }
    else msg(MSG_WARN,"timestamp not on mf boundary, discarded, mfc:%hd,secs:%ld",tstamp->mfc_num,tstamp->secs);
}


void DC_DASCmd(unsigned char type, unsigned char number) {

if (minfrow) msg(MSG_WARN,"DASCmd not on mf boundary, accepted");
minfrow=0;
	
    switch(type) {
	case DCT_TM:
	    switch(number) {
		case DCV_TM_SUSLOG: logging=0;
		case DCV_TM_END: CLOSEFILE; break;
		case DCV_TM_RESLOG: logging=1;
		case DCV_TM_START: break;
		case DCV_TM_CLEAR: break;
	    }
	break;
	default: break;
    }
}


void DC_other(unsigned char *msg_ptr, pid_t sent_tid) {
	/* nut 'n honey */
	return;
}
