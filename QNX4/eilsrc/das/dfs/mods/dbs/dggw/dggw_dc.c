/* 
*/

/* includes */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <globmsg.h>
#include <das.h>
#include <eillib.h>
#include <gw.h>
#include <dfs_mod.h>
#include <rational.h>
#include "dggw_defs.h"

/* global variables */
extern char *minors;	    /* space for 3 minor frames */
extern pid_t tim_proxy;	    /* timer proxy */
extern rational mfsec;	    /* minor frames per sec rational */
extern rational tr;		    /* for rowlets increment and decrement */
extern int limit;

char status[3] = {D,D,D}; /* status of minor frames in 'minors' */
int a = 0;			    /* indices into status */
int b = 1;
int c = 2;
tstamp_type *tsp[3];		/* ptrs to stamps for 3 minor frames */
tstamp_type ts[3];  		/* stamps for 3 minor frames */
short rowlets;
short ts_checks;
int errcount;
static enter_hunt = 1;

/* called when data is received from the dbr */
void DC_data(dbr_data_type *dr_data) {
/* put into buffer */
unsigned char *ptr1, *ptr2, *p;
static unsigned short next_minor_frame;
static tstamp_type newts;
static int s_bytes;
unsigned short mfc;
int r_bytes;
time_t rtime, dt;
unsigned int dmfc;
int tmp;

if (dr_data->n_rows > dbr_info.nrowminf) msg(MSG_EXIT_ABNORM,"Buffer Overflow");

/* look for synch if needed */
for (p=dr_data->data+dr_data->n_rows*tmi(nbrow), ptr1 = dr_data->data,
	 ptr2 = dr_data->data + 1;
		 enter_hunt && ptr2 != p; ptr1++, ptr2++) {
	if ((*ptr1==(tmi(synch) & 0xFF) && *ptr2 == (tmi(synch) >> 8)) ||
	    (tmi(isflag) && *ptr1==(tmi(synch)>>8) && *ptr2==tmi(synch) & 0x0F)) {
	    enter_hunt = 0;
	    s_bytes = 0;
	    ptr1 +=2;
	    msg(MSG,"found synch");
	    if (ptr1 == p) return;
	}
}
assert(!(UNAPPROVED(a) && UNAPPROVED(b) && UNAPPROVED(c)));
/* check for overflow */
if (!s_bytes)
	if ( !DISCARDED(c)) {
	    if (DISCARDED(a)) {
			/* kick out slot A */
			tmp = a; a = b; b = c; c = tmp;
	    }
	    else if (DISCARDED(b)) {
			/* kick out slot B */
			tmp = b; b = c; c = tmp;
	    }
	    else assert(APPROVED(a) || APPROVED(b) || APPROVED(c));
	}

DISCARD(c);

assert(s_bytes!=tmi(nbminf));
r_bytes = abs(p-ptr1);
memcpy(minors + c * tmi(nbminf) + s_bytes, ptr1, r_bytes);
s_bytes+=r_bytes;
if (s_bytes != tmi(nbminf)) return;

/* deal with whole minor frame */
s_bytes = 0;
mfc = getmfc(minors + (c * tmi(nbminf)));
UNAPPROVE(c);

if (!check_synch(minors + (c * tmi(nbminf)))) {
	msg(MSG_WARN,"lost synch");
	DISCARD(c);
	enter_hunt = 1;
	next_minor_frame++;
	return;
}

/* time stamp considerations */
if (ts_checks & TSCHK_REQUIRED && ts_checks & TSCHK_RTIME)
	next_minor_frame = mfc;

if (mfc == next_minor_frame) APPROVE(c);
else ts_checks |= TSCHK_IMPLICIT | TSCHK_CHECK;

if (mfc - newts.mfc_num > TS_MFC_LIMIT)
	ts_checks |= TSCHK_IMPLICIT | TSCHK_REQUIRED;

if (ts_checks) {
	if (ts_checks & TSCHK_RTIME)
	    rtime = time(NULL);
	if (ts_checks & (TSCHK_IMPLICIT|TSCHK_CHECK)) {
	    dmfc = mfc - newts.mfc_num;
	    dmfc /= mfsec.num;
	    ts[c].secs = newts.secs + dmfc * mfsec.den;
	    ts[c].mfc_num = newts.mfc_num + dmfc * mfsec.num;
	    if (ts_checks & TSCHK_CHECK) {
		/* compare rtime to ts[c].secs */
		dmfc = mfc - ts[c].mfc_num;
		dt = dmfc * mfsec.den / mfsec.num;
		dt = rtime - dt - ts[c].secs;
		if (dt > SECDRIFT || dt < -SECDRIFT)
		    ts_checks |= TSCHK_REQUIRED;
		else APPROVE(c);
	    }
	}
	if (ts_checks & TSCHK_REQUIRED) {
	    if (ts_checks & TSCHK_RTIME) {
		ts[c].secs = rtime;
		ts[c].mfc_num = mfc;
	    } /* else ts[c] already set */
	    tsp[c] = &ts[c];
	} else tsp[c] = NULL;
} else tsp[c] = NULL;

assert(APPROVED(c) || UNAPPROVED(c));

if (UNAPPROVED(c)) {
	if (UNAPPROVED(b)) {
	    /* compare between 3 and reset */
	    if (mfc == getmfc(minors + (b * tmi(nbminf))) + 1) {
		APPROVE(b);
		APPROVE(c);
		if (tsp[b] && tsp[c]) tsp[c] = NULL;
		if (UNAPPROVED(a)) DISCARD(a);
	    } else if (mfc == getmfc(minors + (a * tmi(nbminf))) + 2) {
		APPROVE(c);
		DISCARD(b);
		if (UNAPPROVED(a)) APPROVE(a);
	    }
	    else {
		if (UNAPPROVED(a)) DISCARD(a);
		next_minor_frame++;
	    }
	} else next_minor_frame++;
}

if (APPROVED(c)) {
	if (UNAPPROVED(b))  DISCARD(b);
	if (UNAPPROVED(a))  DISCARD(a);
	next_minor_frame=mfc+1;
	rowlets += tr.den;
	if (ts_checks & TSCHK_REQUIRED) newts = ts[c];
}
ts_checks = 0;
}

/* called when a time stamp received from dbr */
void DC_tstamp( tstamp_type *tstamp )  {
/* ignore */
}

/* called when a DASCmd is received from dbr */
void DC_DASCmd(unsigned char type, unsigned char number) {
/* ignore */
}

reply_type DC_other(unsigned char *msg_ptr, int who) {
if (*msg_ptr == DATA) {
	*msg_ptr = DCDATA;
	return(DAS_NO_REPLY);
} else return(GW_dc_other(who));
}

