/*
    User supplied functions for a DG; for file descriptor in.
*/
	
/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/kernel.h>
#include "eillib.h"
#include "das.h"
#include "dbr.h"
#include "dbr_mod.h"
#include "globmsg.h"
#include "rational.h"
#include "fdin.h"

/* defines */
#define TSCHK_RTIME 1
#define TSCHK_IMPLICIT 2
#define TSCHK_CHECK 4
#define TSCHK_REQUIRED 8
#define D 0
#define A 1
#define U 2
#define DISCARD(x) status[x] = D
#define APPROVE(x) status[x] = A
#define UNAPPROVE(x) status[x] = U
#define DISCARDED(x) (status[x] == D)
#define APPROVED(x) (status[x] == A)
#define UNAPPROVED(x) (status[x] == U)
#define UNASSIGNED(x) ((x) < 0)
#define UNASSIGN(x) (x=-1)

/* global variables */
extern char *minors;	    /* space for 3 minor frames */
extern int fd;		    /* descriptor to read from */
extern pid_t tim_proxy;	    /* timer proxy */
extern rational mfsec;	    /* minor frames per sec rational */
extern rational tr;	    /* for rowlets increment and decrement */
extern int limit;
extern int quitter;

/* static variables */
static char status[3] = {D,D,D}; /* status of minor frames in 'minors' */
static int a = 0;		 /* indices into status */
static int b = 1;
static int c = 2;
static tstamp_type *tsp[3];	/* ptrs to stamps for 3 minor frames */
static tstamp_type ts[3];  	/* stamps for 3 minor frames */
static enter_hunt = 1;
static int not_armed;
static int errcount;
static int minf_row;
static short rowlets;
static short ts_checks;
static int done;

int DG_get_data( token_type n_rows ) {
  static int s_rows;		/* sent rows of a frame */
  unsigned char *ptr1, *ptr2;
  int nrows1, nrows2;
  int af1, af2;
  int get, got;

  if (done) {			/* end of file, minor frame boundary */
    if (!quitter) DG_s_dascmd(DCT_TM,DCV_TM_END);
    else DG_s_dascmd(DCT_QUIT,DCV_QUIT);
    return 1;
  }

  nrows1 = nrows2 = 0;
  ptr1 = ptr2 = NULL;

  /* send up to 2 whole minor frames */
  get = (n_rows > 2) ? 2 : n_rows;

  for (UNASSIGN(af1), got=0; got < get;) {
    Collect_data();
    if (APPROVED(a)) {
      af1 = a;
      got++;
    }
    if (APPROVED(b)) {
      if (UNASSIGNED(af1)) af1 = b;
      got++;
    }
    if (APPROVED(c)) {
      if (UNASSIGNED(af1)) {
	af1 = c;
	got++;
      }
      else if (APPROVED(b)) got++;
    }
  }

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
  if (get != 1) {
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

int DG_DASCmd( unsigned char type, unsigned char num ) {
  /* called when DG recieves a DASCmd */	
  switch(type) {
  case DCT_TM:
    switch(num) {
    case DCV_TM_SUSLOG: break;
    case DCV_TM_END: break;
    case DCV_TM_RESLOG: break;
    case DCV_TM_START:
      done = 0;
      ts_checks = TSCHK_REQUIRED | TSCHK_RTIME;
      break;
    case DCV_TM_CLEAR: errcount = 0; break;
    default: msg(MSG_WARN,"received unknown DASCmd with type %d, num %d",type,num);
      break;
    }
    break;
  case DCT_QUIT: break;
  default: msg(MSG_WARN,"received unknown DASCmd with type %d, num %d",type,num);
    break;
  }
  return 1;
}

static void ts_check(void) {
  rowlets -= tr.num;
  if (rowlets < -limit || rowlets > limit)
    ts_checks = TSCHK_RTIME | TSCHK_CHECK;
}

static void Collect_data(void) {
  static char byt1, byt2;
  static unsigned short next_minor_frame;
  static tstamp_type newts;
  static int s_bytes;
  unsigned short mfc;
  int r_bytes;
  time_t rtime, dt;
  unsigned int dmfc;
  int tmp;

  if (!done) {

    /* look for synch if needed */
    if (enter_hunt)
      if ( (r_bytes = read(fd,&byt2,1)) == -1)
	msg(MSG_EXIT_ABNORM,"Can't read a byte from");
      else if (!r_bytes) done = 1;
      else if ( (byt1 == (tmi(synch) & 0xFF) && byt2 == (tmi(synch) >> 8)) ||
	       (tmi(isflag) && byt1 == (tmi(synch) >> 8) && byt2 == tmi(synch) & 0x0F)) {
	enter_hunt = 0;
	byt1 = 0;
	s_bytes = 0;
	msg(MSG,"found synch");
      } else byt1 = byt2;

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
    if ( (r_bytes = read(fd, minors + c * tmi(nbminf) + s_bytes, tmi(nbminf)-s_bytes)) != tmi(nbminf)-s_bytes) {
      if (r_bytes == -1) msg(MSG_EXIT_ABNORM,"error reading from");
      else if (!r_bytes) done = 1;
      else s_bytes+=r_bytes;
      return;
    }

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
	}			/* else ts[c] already set */
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
}

int DG_other(unsigned char *msg_ptr, pid_t sent_tid) {
  /* called when a message is recieved which is not handled by the distributor */
  reply_type r = DAS_UNKN;

  if (sent_tid==tim_proxy) ts_check();
  else {
    Reply(sent_tid,&r,sizeof(reply_type));
    msg(MSG_WARN,"received UNKNOWN msg from process %d",sent_tid);
  }
  return 1;
}



