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
union home_row {
  struct {
    CURR I;
    TEMP Thtr;
    RES3 R;
    UINT Synch;
  } U0;
  struct {
    char U2[4];
    RES2 Rt;
  } U1;
  struct {
    char U4[4];
    CAP2 Ct;
  } U3;
  struct {
    char U6[4];
    TEMP Tamb;
  } U5;
} *home;


void data_queue::tminitfunc() {
}
static void nullfunc(void);
static void CF1_0(void) {
  {
    double dT, Vc;
    Vc =(Thtr*1E-2) -(Tamb*1E-2);
    dT = ((I*1E-3) *(I*1E-3) *(R*1E-3) -
  			  Vc/(Rt*1E-2) )/(4 *(Ct*1E-2));
    Thtr = (short)(( Vc + dT ) * 100 + Tamb + .5);
  }
  { I = (short)(HtrData.I * 1000); }
  home->U0.Thtr = Thtr;
  home->U0.I = I;
}

static void BF4_0(void) {
  CF1_0();
  { Tamb = (short)(HtrData.Tamb * 100); }
  home->U5.Tamb = Tamb;
}

static void BF16_1(void) {
  CF1_0();
  { Ct = (short)(HtrData.Ct * 100); }
  home->U3.Ct = Ct;
}

static void BF16_2(void) {
  CF1_0();
  { R = (short)(HtrData.R * 1000); }
  home->U0.R = R;
}

static void BF16_3(void) {
  CF1_0();
  { Rt = (short)(HtrData.Rt * 100); }
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
#define LCMMN 16
#define ROLLOVER_MFC 0
#define NROWMAJF 16
#define MFSECNUM 4
#define MFSECDEN 1
#define SECDRIFT 90
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

#define incmod(x,y) if (x==((y)-1)) x = 0; else x++

//static short rowlets;

#if (NROWMINF == 1)
  #define MINF_ROW 0
  #define MINF_ROW_ZERO
  #define MINF_ROW_INC
#else
  #define MINF_ROW collector::minf_row
  #define MINF_ROW_ZERO collector::minf_row = 0
  #define MINF_ROW_INC ++collector::minf_row
#endif
unsigned short collector::minf_row = 0;
unsigned short collector::majf_row = 0;

// %data_defs%>

// <%main_program%

// ### Make collector a #define and create a subclass
// ### for subbus that overrides the event(dq_event_stop) and
// ### Collect_row() methods
int main(int argc, char **argv) {
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
void collector::ts_check() {
  rowlets -= TRN;
  if (rowlets < LOWLIM || rowlets > HIGHLIM)
	ts_checks = TSCHK_RTIME | TSCHK_CHECK;
}

/**
 * Should come up with a test to guarantee that the right thing happens in all circumstances. 
 */
void collector::Collect_Row() {
  time_t rtime;
  long dt;
  
  #ifdef _SUBBUS_H
	tick_sic(); // probably implement this through inheritance
  #endif
  if (ts_checks & TSCHK_RTIME) {
    rtime = time(NULL);
    // It's only reasonable to check realtime at even second boundaries
    // This check assumes tm_info.t_stmp.mfc_num % MFSECNUM == 0
    if ((ts_checks & TSCHK_CHECK) && next_minor_frame%MFSECNUM == 0) {
      dt = (next_minor_frame - tm_info.t_stmp.mfc_num)/MFSECNUM;
      dt *= MFSECDEN;
      dt = rtime - dt - tm_info.t_stmp.secs;
      if (dt > SECDRIFT || dt < -SECDRIFT)
        ts_checks |= TSCHK_REQUIRED;
    }
  }
  if ((ts_checks & TSCHK_RTIME) && (ts_checks & TSCHK_REQUIRED)) {
    next_minor_frame = next_minor_frame % LCMMN;
    commit_tstamp( next_minor_frame, rtime );
  } else if ( next_minor_frame == 0 ) {
    //m* = (2^16)%lcm(M,n)
    //m1 = 0
    //t1 = t0 + d(2^16 - m* - m0)/n
    next_minor_frame = ROLLOVER_MFC;
    commit_tstamp( 0, tm_info.t_stmp.secs +
      MFSECDEN * ((long)USHRT_MAX - tm_info.t_stmp.mfc_num - next_minor_frame + 1) /
        MFSECNUM );
  } else if ( next_minor_frame - tm_info.t_stmp.mfc_num > TS_MFC_LIMIT) {
    // q = floor((m-m0)/n)
    // m1 = m0+q*n
    // t1 = t0 + d*(m1-m0)/n = t0 + d*q
    unsigned short q = (next_minor_frame - tm_info.t_stmp.mfc_num)/MFSECNUM;
    commit_tstamp( tm_info.t_stmp.mfc_num + q * MFSECNUM,
        tm_info.t_stmp.secs + MFSECDEN * q );
  }
  ts_checks = 0;
  MFCtr = next_minor_frame;
  next_minor_frame++;
  if ( NROWMINF == 1 || MINF_ROW == NROWMINF-1 ) {
    /* Last row of minor frame: Synch Calculations */
    if ( INVSYNCH && collector::majf_row == NROWMAJF-1)
      Synch = ~SYNCHVAL;
    else
      Synch = SYNCHVAL;
    MINF_ROW_ZERO;
  } else MINF_ROW_INC;
  
  /* appropriate collection function */
  home = (union home_row *) row[last];
  efuncs[collector::majf_row]();
  incmod(collector::majf_row, NROWMAJF);
  rowlets += TRD;
  commit_rows(MFCtr, MINF_ROW, 1);
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
  #define ROWS(x) (((unsigned long)(x))*NROWMINF+MINF_ROW)
  #define FRACSECS(x) (((unsigned long)ROWS(x))*NSECSPER)

  long itime(void) {
	  return(tm_info.t_stmp.secs +
		FRACSECS(MFCtr-tm_info.t_stmp.mfc_num) / NROWSPER );
  }
  double dtime(void) {
	  return(tm_info.t_stmp.secs +
		(double) FRACSECS(MFCtr-tm_info.t_stmp.mfc_num) / NROWSPER );
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
