/* Hand-edited version of compiled hsimcol.c */
// <%headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include "nortlib.h"
#include "oui.h"
#include "tm.h"
#include "Collector.h"

// %headers%>

  #include "htrsim.h"


 typedef short CURR;
 typedef unsigned short RES3;
 typedef unsigned short RES2;
 typedef unsigned short CAP2;
 typedef short TEMP;
 typedef unsigned short UINT;

// <%console_functions%
// ### This stuff almost certainly needs to change
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
// %console_functions%>

CURR I;
RES3 R;
RES2 Rt;
CAP2 Ct;
TEMP Tamb;
TEMP Thtr;
UINT Synch, MFCtr;
union {
  struct {
    UINT MFCtr;
    CURR I;
    TEMP Thtr;
    RES3 R;
    UINT Synch;
  } U0;
  struct {
    char U2[6];
    RES2 Rt;
  } U1;
  struct {
    char U4[6];
    CAP2 Ct;
  } U3;
  struct {
    char U6[6];
    TEMP Tamb;
  } U5;
} *home;

static void nullfunc(void);
static void CF1_0(void) {
  {
    double dT, Vc;
    Vc =(Thtr*1E-2) -(Tamb*1E-2);
    dT = ((I*1E-3) *(I*1E-3) *(R*1E-3) -
  			  Vc/(Rt*1E-2) )/(4 *(Ct*1E-2));
    Thtr = ( Vc + dT ) * 100 + Tamb + .5;
  }
  { I = HtrData.I * 1000; }
  home->U0.Thtr = Thtr;
  home->U0.I = I;
  home->U0.MFCtr = MFCtr;
  home->U0.Synch = Synch;
}

static void BF4_0(void) {
  CF1_0();
  { Tamb = HtrData.Tamb * 100; }
  home->U5.Tamb = Tamb;
}

static void BF16_1(void) {
  CF1_0();
  { Ct = HtrData.Ct * 100; }
  home->U3.Ct = Ct;
}

static void BF16_2(void) {
  CF1_0();
  { R = HtrData.R * 1000; }
  home->U0.R = R;
}

static void BF16_3(void) {
  CF1_0();
  { Rt = HtrData.Rt * 100; }
  home->U1.Rt = Rt;
}

static void (*efuncs[16])() = {
  BF4_0,
  BF16_1,
  BF16_2,
  BF16_3,
  BF4_0,
  CF1_0,
  CF1_0,
  CF1_0,
  BF4_0,
  CF1_0,
  CF1_0,
  CF1_0,
  BF4_0,
  CF1_0,
  CF1_0,
  CF1_0
};

#define TRN 40
#define TRD 1
#define LOWLIM (-360)
#define HIGHLIM (360)
#define NBROW 10
#define NROWMINF 1
#define NSECSPER 1
#define NROWSPER 4
#define SYNCHVAL 0xABB4
#define NROWMAJF 16
#define MFSECNUM 4
#define MFSECDEN 1
#define SECDRIFT 90
// ### Added ROLLOVER and INVSYNCH definitions
#define ROLLOVER 0
#define INVSYNCH 0

// <%data_defs%
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
// %data_defs%>

// <%main_program%

// ### Make collector a #define and create a subclass
// ### for subbus that overrides the event(dq_event_stop) and
// ### Collect_row() methods
void main(int argc, char **argv) {
  // oui_init_options(argc, argv);
  collector col;
  col.init();
  col.operate();
  return 0;
}
// %main_program%>

// <%pre_other
/**
 * Called from a slow timer to make sure we aren't drifting.
 */
static void ts_check(void) {
  rowlets -= TRN;
  if (rowlets < LOWLIM || rowlets > HIGHLIM)
	ts_checks = TSCHK_RTIME | TSCHK_CHECK;
}

void collector::Collect_row() {
  time_t rtime, dt;
  unsigned int dmfc;
  
  #ifdef _SUBBUS_H
	tick_sic();
  #endif
  if (NROWMINF == 1 || minf_row == 0) {
    if ((ts_checks & TSCHK_REQUIRED) == 0 && (
          next_minor_frame == ROLLOVER ||
          next_minor_frame - curts->mfc_num > TS_MFC_LIMIT))
      ts_checks |= TSCHK_IMPLICIT | TSCHK_REQUIRED;
    if (ts_checks) {
      newts = &tstamps[next_ts_index];
      if (newts == curts)
        nl_error(MSG_EXIT_ABNORM, "TimeStamp overflow");
      if (ts_checks & TSCHK_RTIME)
        rtime = time(NULL);
      if (ts_checks & (TSCHK_IMPLICIT|TSCHK_CHECK)) {
        /* mfc rate is MFSECNUM/MFSECDEN mfcs/second, so mfcs/MFSECNUM = secs/MFSECDEN = dmfc   */
        dmfc = (next_minor_frame - curts->mfc_num)/MFSECNUM;
        newts->secs = curts->secs + dmfc * MFSECDEN;
        if (MFSECNUM != 1)
          newts->mfc_num = curts->mfc_num + dmfc*MFSECNUM;
        else
          newts->mfc_num = next_minor_frame;
        if (ts_checks & TSCHK_CHECK) {
          /* compare rtime to newts->secs */
          if (MFSECNUM != 1) {
            dmfc = next_minor_frame - newts->mfc_num;
            dt = dmfc * MFSECDEN / MFSECNUM;
            dt = rtime - dt - newts->secs;
          } else {
            dt = rtime - newts->secs;
          }
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
        if (next_minor_frame == ROLLOVER) {
        	next_minor_frame = 0;
        	newts->mfc_num -= ROLLOVER;
        }
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
    if (NROWMINF != 1)
      minf_row = 1;
  } else if (minf_row == NROWMINF-1) {
    /* Last row of minor frame: Synch Calculations */
    if ( INVSYNCH && majf_row == NROWMAJF-1)
      Synch = ~SYNCHVAL;
    else
      Synch = SYNCHVAL;
    if (NROWMINF != 1)
      minf_row = 0;
  } else ++minf_row;
  
  /* appropriate collection function */
  home = (void *) dpdata[col_row];
  efuncs[majf_row]();
  incmod(majf_row, NROWMAJF);
  rowlets += TRD;
  
  incmod(col_row, DP_NROWS);
  if (col_row == trans_row)
	nl_error(MSG_EXIT_ABNORM, "Collection overflow");
}

// %pre_other%>

// <%COL_send%
// %COL_send%>

// <%DG_other_decls%
// %DG_other_decls%>
  
// <%DG_other_cases%
// %DG_other_cases%>

// <%Rest_of_the_file

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

// %Rest_of_the_file%>

tm_info_t tm_info = {
   /* version: '6.0' */
   54,  46,  48,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,
   /* md5: */
  0xCB, 0xA9, 0xFA, 0x7D, 0x8D, 0x16, 0x5A, 0xC7,
  0xC7, 0x6A, 0x07, 0x6B, 0x69, 0xC2, 0xFA, 0x3F,
  10, /* NBMINF */
  10, /* NBROW */
  16, /* NROWSMAJF */
  1, /* NSECSPER */
  4, /* NROWSPER */
  0, 1, /* MFC lsb col, msb col */
  0xABB4, /* Synch Value */
  0 /* not inverted */
};
