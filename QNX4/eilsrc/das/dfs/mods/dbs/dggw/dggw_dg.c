/* 
*/

/* includes */
#include <globmsg.h>
#include <assert.h>
#include <eillib.h>
#include <gw.h>
#include <rational.h>
#include "dggw_defs.h"

/* global variables */
extern char *minors;	    /* space for 3 minor frames */
extern pid_t tim_proxy;	    /* timer proxy */
extern rational mfsec;	    /* minor frames per sec rational */
extern rational tr;		    /* for rowlets increment and decrement */
extern int limit;

extern char status[];	/* status of minor frames in 'minors' */
extern int a;			/* indices into status */
extern int b;
extern int c;
extern tstamp_type *tsp[];		/* ptrs to stamps for 3 minor frames */
extern tstamp_type ts[];  		/* stamps for 3 minor frames */
extern short rowlets;
extern short ts_checks;
extern int errcount;

static void ts_check(void) {
  rowlets -= tr.num;
  if (rowlets < -limit || rowlets > limit)
	ts_checks = TSCHK_RTIME | TSCHK_CHECK;
}

void DG_DASCmd(unsigned char type, unsigned char val) {
/* called when DG recieves a DASCmd */	
    switch(type) {
	case DCT_TM:
	    switch(val) {
			case DCV_TM_SUSLOG: break;
			case DCV_TM_END: break;
			case DCV_TM_RESLOG: break;
			case DCV_TM_START:
			    ts_checks = TSCHK_REQUIRED | TSCHK_RTIME;
			    break;
			case DCV_TM_CLEAR: errcount = 0; break;
			default: msg(MSG_WARN,"received unknown DASCmd with type %d, val %d",type,val);
			    break;
		    }
		    break;
		case DCT_QUIT: break;
		default: msg(MSG_WARN,"received unknown DASCmd with type %d, val %d",type,val);
		    break;
    }
return;
}

reply_type DG_other(unsigned char *msg_ptr, pid_t who) {
if (who==tim_proxy) {
	ts_check();
	return(DAS_NO_REPLY);
}	
return(GW_dg_other(who,msg_ptr));
}

/* Called when data buffer receives an order for data from its clients. */
int DG_get_data(token_type n_rows) {
static int s_rows; /* sent rows of a frame */
unsigned char *ptr1, *ptr2;
int nrows1, nrows2;
int af1, af2;

nrows1 = nrows2 = 0;
ptr1 = ptr2 = NULL;

/* send up to 2 whole minor frames */
/* find first approved frame */
/* return if no data */
if (UNAPPROVED(a)) return 1;
if (APPROVED(a)) af1 = a;
else if (UNAPPROVED(b)) return 1;
    else if (APPROVED(b)) af1 = b;
	else if (UNAPPROVED(c)) return 1;
	    else if (APPROVED(c)) af1 = c;
		else return 1;

if (!s_rows) {
    if (tsp[af1]==NULL)
	ptr1 = minors + (af1 * tmi(nbminf));
}
else ptr1 = minors + (af1 * tmi(nbminf)) + (s_rows * tmi(nbrow));

/* send a timestamp */
if (!ptr1) {
    DG_s_tstamp(tsp[af1]);
    tsp[af1] = NULL;
	return 1;
}

nrows1 = min(dbr_info.nrowminf-s_rows,min(n_rows,dbr_info.nrowminf));
assert(nrows1);

/* find next approved frame */
if (af1 != 1) {
    if (af1 == a) af2 = b;
    if (af1 == b) af2 = c;
    if (DISCARDED(af2) && af2 == b) af2 = c;
    if (APPROVED(af2) && tsp[af2] == NULL) {
		ptr2 = minors + (af2 * tmi(nbminf));
		nrows2 = min(dbr_info.nrowminf, n_rows - nrows1);
    }
}

assert(ptr1 != NULL);

/* calculate new s_rows */
if (ptr1 && !ptr2) {
    s_rows += nrows1;
    if (s_rows==dbr_info.nrowminf) DISCARD(af1);
}
else {
    DISCARD(af1);
    s_rows = nrows2;
    if (s_rows==dbr_info.nrowminf) DISCARD(af2);
}
if (s_rows == dbr_info.nrowminf) s_rows = 0;

/* send approved data */
DG_s_data(nrows1, ptr1, nrows2, ptr2);
	
return 1;
}




