/* colmain.skel Skeleton for collection main program
 * $Log$
 * Revision 1.2  2008/07/03 20:58:07  ntallen
 * In the process of testing.
 *
 * Revision 1.1  2008/07/03 15:11:07  ntallen
 * Copied from QNX4 version V1R9
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/timers.h>
#include <sys/proxy.h>
#include <time.h>
#include <unistd.h>
#include "nortlib.h"
#include "oui.h"
#include "tm.h"
#include "msg.h"
#include "globmsg.h"
#include "cc.h"
#include "collect.h"
#include "timerbd.h"

static char cmrcsid[] =
      "$Id$";

 typedef unsigned short UINT;
#ifdef OPT_COL_INIT
  #error OPT_COL_INIT is obsolete
#endif
#ifdef MSG_LABEL
  #error MSG_LABEL is obsolete
#endif

#ifdef N_CONSOLES
  #include "nl_cons.h"
  #ifndef DATA_ATTR
	#define DATA_ATTR 7
  #endif

  #define cdisplay(d,r,c,s) nlcon_display(d,r,c,s,DATA_ATTR)
  #define display_(d,r,c,t,v,s) {\
	static t sv_;\
	if (v != sv_) { cdisplay(d,r,c,s); sv_ = v; }\
  }
#endif

#ifdef CONSOLE_INIT
  #error CONSOLE_INIT is obsolete
#endif


UINT Synch, MFCtr;
UINT Q;
union {
  struct {
    char U1[1];
    UINT Synch;
  } U0;
  struct {
    char U3[1];
  } U2;
} *home;

static void nullfunc(void);
static void CF1_0(void) {
}

static void BF4_0(void) {
  CF1_0();
  { Q = MFCtr; }
  memcpy(home->U2.U3, &Q, 1);
}

static void BF4_1(void) {
  CF1_0();
  memcpy(home->U0.U1, ((char*)(&Q))+1, 1);
}

static void (*efuncs[4])() = {
  BF4_0,
  BF4_1,
  CF1_0,
  CF1_0
};

#define TRN 50
#define TRD 1
#define LOWLIM (-450)
#define HIGHLIM (450)
#define NBROW 5
#define NROWMINF 1
#define NSECSPER 1
#define NROWSPER 5
#define LCMMN 20
#define ROLLOVER_MFC 16
#define SYNCHVAL 0xABB4
#define INVSYNCH 0
#define NROWMAJF 4
#define MFSECNUM 5
#define MFSECDEN 1
#define SECDRIFT 90

/* Some temporary defs until everything is in the right place */
#ifndef TS_MFC_LIMIT
  #define TS_MFC_LIMIT 32767
#endif

/* This is for real: */
#ifndef DG_OTHER_CASES
  #define DG_OTHER_CASES
#endif

/* for debugging */
int check_ts = 1;

#if (NROWMAJF < 16)
  #define DP_NROWS 16
#else
  #define DP_NROWS NROWMAJF
#endif
#define NTS 3
#define incmod(x,y) if (x==((y)-1)) x = 0; else x++

static unsigned char dpdata[DP_NROWS][NBROW];
static tstamp_type tstamps[NTS], *tsps[DP_NROWS], *curts;
static unsigned short next_ts_index;
static unsigned short trans_row, col_row;
static short rowlets;
static short ts_checks;
#ifdef NEED_TIME_FUNCS
  static unsigned short CurMFC;
#endif
#define TSCHK_RTIME 1
#define TSCHK_IMPLICIT 2
#define TSCHK_CHECK 4
#define TSCHK_REQUIRED 8
static unsigned short next_minor_frame, majf_row;
#if (NROWMINF != 1)
  static unsigned short minf_row;
#endif
#ifndef DG_CUSTOM_INIT
  #define DG_CUSTOM_INIT
#endif
#ifndef TMINITFUNC
  #define TMINITFUNC
#endif
#ifndef ONINT_INIT
  #define ONINT_INIT
#endif
#ifndef ONINT_TERM
  #define ONINT_TERM
#endif
#ifndef DG_CUSTOM_TERM
  #define DG_CUSTOM_TERM
#endif


void main(int argc, char **argv) {
  oui_init_options(argc, argv);
  { int nlrsp;
	
	nlrsp = set_response(NLRSP_QUIET);
	if (find_CC(0) != -1)
	  cc_init_options(argc, argv, DCT_TM, DCT_TM, 0, 0, FORWARD_QUIT);
	set_response(nlrsp);
  }
  DG_CUSTOM_INIT;
  TMINITFUNC;
  BEGIN_MSG;
  DG_operate();
  DG_CUSTOM_TERM;
  nl_error(MSG_EXIT_NORM, "Terminated");
}


static void ts_check(void) {
  rowlets -= TRN;
  if (rowlets < LOWLIM || rowlets > HIGHLIM)
	ts_checks = TSCHK_RTIME | TSCHK_CHECK;
}

static void Collect_row(void) {
  time_t rtime, dt;
  unsigned int dmfc;
  tstamp_type *newts;
  
  #ifdef _SUBBUS_H
	tick_sic();
  #endif
  #if (NROWMINF != 1)
	if (minf_row == 0) {
  #endif
	  if ((ts_checks & TSCHK_REQUIRED) == 0 && (
	  #ifdef ROLLOVER
		  next_minor_frame == ROLLOVER ||
	  #endif
		  next_minor_frame - curts->mfc_num > TS_MFC_LIMIT))
		ts_checks |= TSCHK_IMPLICIT | TSCHK_REQUIRED;
	  if (ts_checks) {
		newts = &tstamps[next_ts_index];
		if (newts == curts)
		  nl_error(MSG_EXIT_ABNORM, "TimeStamp overflow");
		if (ts_checks & TSCHK_RTIME)
		  rtime = time(NULL);
		if (ts_checks & (TSCHK_IMPLICIT|TSCHK_CHECK)) {
		  /* mfc rate is MFSECNUM/MFSECDEN mfcs/second, so
		     mfcs/MFSECNUM = secs/MFSECDEN = dmfc
		  */
		  #if (MFSECDEN == 1)
			#define MFSECDENMUL
		  #else
			#define MFSECDENMUL * MFSECDEN
		  #endif
		  dmfc = next_minor_frame - curts->mfc_num;
		  #if (MFSECNUM != 1)
			dmfc /= MFSECNUM;
		  #endif
		  newts->secs = curts->secs + dmfc MFSECDENMUL;
		  #if (MFSECNUM != 1)
			newts->mfc_num = curts->mfc_num + dmfc*MFSECNUM;
		  #else
			newts->mfc_num = next_minor_frame;
		  #endif
		  if (ts_checks & TSCHK_CHECK) {
			/* compare rtime to newts->secs */
			#if (MFSECNUM != 1)
			  dmfc = next_minor_frame - newts->mfc_num;
			  dt = dmfc MFSECDENMUL / MFSECNUM;
			  dt = rtime - dt - newts->secs;
			#else
			  dt = rtime - newts->secs;
			#endif
			if (dt > SECDRIFT || dt < -SECDRIFT)
			  ts_checks |= TSCHK_REQUIRED;
		  }
		}
		if (ts_checks & TSCHK_REQUIRED) {
		  if (ts_checks & TSCHK_RTIME) {
			/* New time stamp:
			  If higher resolution time is available {
				f = fraction of a second extra
				rmfc = rate of mfc Hz
				f*rmfc = number of minor frames since even second.
				record floor(seconds) and mfc - f*rmfc
			  }
			*/
			newts->secs = rtime;
			newts->mfc_num = next_minor_frame;
		  }
		  #ifdef ROLLOVER
			if (next_minor_frame == ROLLOVER) {
			  next_minor_frame = 0;
			  newts->mfc_num -= ROLLOVER;
			}
		  #endif
		  incmod(next_ts_index, NTS);
		  tsps[col_row] = curts = newts;
		} else tsps[col_row] = NULL;
		ts_checks = 0;
	  } else tsps[col_row] = NULL;
	  MFCtr = next_minor_frame;
	  #ifdef NEED_TIME_FUNCS
		CurMFC = MFCtr;
	  #endif
	  next_minor_frame++;
  #if (NROWMINF != 1)
	  minf_row = 1;
    } else
	  #if (NROWMINF > 2)
		if (minf_row == NROWMINF-1)
	  #endif
  #endif
	{ /* Synch Calculations */
      #ifdef INVSYNCH
		if (majf_row == NROWMAJF-1)
		  Synch = ~SYNCHVAL;
		else
	  #else
		  Synch = SYNCHVAL;
	  #endif
  #if (NROWMINF != 1)
	  minf_row = 0;
  #endif
	}
  #if (NROWMINF > 2)
	  else ++minf_row;
  #endif
  
  /* appropriate collection function */
  home = (void *) dpdata[col_row];
  efuncs[majf_row]();
  incmod(majf_row, NROWMAJF);
  rowlets += TRD;
  
  incmod(col_row, DP_NROWS);
  if (col_row == trans_row)
	nl_error(MSG_EXIT_ABNORM, "Collection overflow");
}

/* time_prox holds information pertinent to the two timing proxies.
   If proxy == 0, the proxy has not been initialized.
   If proxy != 0, timer_type indicates what type of timer is used.
   TPTYPE_SYS means u.timer is a system timer.
   TPTYPE_TBD means u.tmrno is a timer board timer.
   TPTYPE_EXT means an external timing source is used.
*/
struct time_prox {
  pid_t proxy;
  int timer_type;
  union {
	timer_t timer;
	int tmrno;
  } u;
};
#define TPTYPE_SYS 0
#define TPTYPE_TBD 1
#define TPTYPE_EXT 2
static struct time_prox col_timer = {0,0}, ts_timer = {0,0};

#define NSEC (1000000000)

static void start_timing(int rn, int rd, struct time_prox *tp,
						 unsigned char msgtxt) {
  struct itimercb tcb;
  struct itimerspec tval, otval;

  assert(tp->proxy == 0);
  if (rn <= 4*rd) {
	set_response(0);
	tp->u.tmrno = Tmr_proxy(3, rn*(TMR_0_FREQ/rd), msgtxt);
	set_response(3);
	if (tp->u.tmrno == -1) nl_error(0, "Using System Timer");
	else {
	  tp->proxy = -1;
	  tp->timer_type = TPTYPE_TBD;
	  nl_error(0, "Using Timer Board Timer");
	  return;
	}
  }
  tp->proxy = nl_make_proxy(&msgtxt, 1);
  tcb.itcb_event.evt_value = tp->proxy;
  tp->u.timer = mktimer(TIMEOFDAY, _TNOTIFY_PROXY, &tcb);
  if (tp->u.timer == -1)
	nl_error(MSG_EXIT_ABNORM, "Error making timer: %d", tp->u.timer);
  tval.it_value.tv_sec = rn/rd;
  tval.it_value.tv_nsec = (rn%rd) * (NSEC/rd);
  tval.it_interval = tval.it_value;
  if (reltimer(tp->u.timer, &tval, &otval) == -1)
	nl_error(MSG_EXIT_ABNORM, "Error in reltimer: %d\n", errno);
  tp->timer_type = TPTYPE_SYS;
}

/* Only detaches proxy if timing is internal */
static void stop_timing(struct time_prox *tp) {
  if (tp->proxy != 0) {
	switch (tp->timer_type) {
	  case TPTYPE_TBD:
		Tmr_reset(tp->u.tmrno);
		tp->proxy = 0;
		break;
	  case TPTYPE_SYS:
		rmtimer(tp->u.timer);
		tp->u.timer = 0;
		if (tp->proxy != 0) {
		  qnx_proxy_detach(tp->proxy);
		  tp->proxy = 0;
		}
		break;
	  default:
		break;
	}
  }
}

#define MKDCTV(x,y) ((x<<8)|y)
#define DCTV(x,y) MKDCTV(DCT_##x,DCV_##y)

int DG_DASCmd(unsigned char type, unsigned char val) {
  switch (MKDCTV(type, val)) {
	case DCTV(TM,TM_START):
	  if (check_ts)
		start_timing(10, 1, &ts_timer, COL_TSPROXY);
	  if (col_timer.proxy == 0)
		start_timing(tmi(nsecsper), tmi(nrowsper), &col_timer, COL_REG_SIG);
	  ts_checks = TSCHK_RTIME | TSCHK_REQUIRED;
	  next_ts_index = 0;
	  curts = NULL;
	  trans_row = col_row = 0;
	  rowlets = 0;
	  next_minor_frame = majf_row = 0;
	  #if (NROWMINF != 1)
		minf_row = 0;
	  #endif
	  ONINT_INIT
	  break;
	case DCTV(TM,TM_END):
	case DCTV(QUIT,QUIT):
	  #ifdef _SUBBUS_H
		disarm_sic();
	  #endif
	  ONINT_TERM
	  stop_timing(&col_timer);
	  stop_timing(&ts_timer);
	  break;
	default:
	  break;
  }
  return(0);
}


int DG_other(unsigned char *msg_ptr, int sent_tid) {
  
  switch (*msg_ptr) {
	case COL_REG_SIG:
	  if (dbr_info.tm_started) Collect_row();
	  break;
	case COL_TSPROXY:
	  if (dbr_info.tm_started) ts_check();
	  break;
	case COL_REGULATE:
	  {
		struct dgreg_rep dgregrep;
		unsigned char msgtxt = COL_REG_SIG;

		if (col_timer.proxy != 0) {
		  dgregrep.reply_code = DAS_BUSY;
		  dgregrep.proxy = col_timer.proxy;
		} else {
		  col_timer.proxy = nl_make_proxy(&msgtxt, 1);
		  col_timer.timer_type = TPTYPE_EXT;
		  dgregrep.proxy = col_timer.proxy;
		  dgregrep.reply_code = DAS_OK;
		}
		Reply(sent_tid, &dgregrep, sizeof(dgregrep));
	  }
	  return(0);
	DG_OTHER_CASES /* defined via preprocessor elsewhere */
	default:
	  return(reply_byte(sent_tid, DAS_UNKN));
  }
  /* We only get here on the two proxy calls: proxies don't need reply */
  return(0);
}

/* Need a 'current time stamp' different from the one in dbr_info,
   since that one pertains to stuff already being transmitted.
   Need to optimize for wrap-around when DG_s_data is augmented
   to support two blocks of data.
*/
int DG_get_data(unsigned int n_rows) {
  int rowlim, i;
  
  if (trans_row != col_row) {
	if (tsps[trans_row] != NULL) {
	  DG_s_tstamp(tsps[trans_row]);
	  tsps[trans_row] = NULL;
	} else {
	  rowlim = trans_row + n_rows;
	  if (DP_NROWS < rowlim) rowlim = DP_NROWS;
	  if (col_row > trans_row && col_row < rowlim) rowlim = col_row;
	  for (i = trans_row; i < rowlim && tsps[i] == NULL; i++);
	  assert(i != trans_row);
	  DG_s_data(i-trans_row, dpdata[trans_row], 0, NULL);
	  trans_row = i;
	  if (trans_row == DP_NROWS) trans_row = 0;
	}
  }
  return(0);
}

#ifdef NEED_TIME_FUNCS
  #if (NROWMINF != 1)
	#define ROWS(x) (((unsigned long)(x))*NROWMINF+minf_row)
  #else
	#define ROWS(x) (x)
  #endif
  #if (NSECSPER != 1)
	#define FRACSECS(x) (((unsigned long)ROWS(x))*NSECSPER)
  #else
	#define FRACSECS(x) ROWS(x)
  #endif
  long itime(void) {
	  return(dbr_info.t_stmp.secs +
		FRACSECS(CurMFC-dbr_info.t_stmp.mfc_num)
	#if (NROWSPER != 1)
		  / NROWSPER
	#endif
		  );
  }
  double dtime(void) {
	  return(dbr_info.t_stmp.secs +
		(double) FRACSECS(CurMFC-dbr_info.t_stmp.mfc_num)
	#if (NROWSPER != 1)
		  / NROWSPER
	#endif
		  );
  }
  double etime(void) {
	double t;
	static double t0 = -1e9;
	
	t = dtime();
	if (t0 == -1e9) t0 = t;
	return(t - t0);
  }
#endif

tm_info_t tm_info = {
   /* version: '6.0' */
   54,  46,  48,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
   /* md5: */
  0xE8, 0xF6, 0x77, 0xEA, 0x52, 0xC7, 0x1D, 0x7D,
  0xC8, 0x57, 0x89, 0x3B, 0xD1, 0xF8, 0x34, 0xC8,
  5, /* NBMINF */
  5, /* NBROW */
  4, /* NROWSMAJF */
  1, /* NSECSPER */
  5, /* NROWSPER */
  0, 1, /* MFC lsb col, msb col */
  0xABB4, /* Synch Value */
  0 /* not inverted */
};
