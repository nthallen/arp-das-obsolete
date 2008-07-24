/* Skeleton headers section */
/* colmain.skel Skeleton for collection main program
 * $Log$
 * Revision 1.9  2008/07/23 21:06:25  ntallen
 * Test photon app
 *
 * Revision 1.3  2008/07/23 17:08:00  ntallen
 * First cut at QNX6 collection skeleton
 *
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
#include <unistd.h>
#include "nortlib.h"
#include "oui.h"
#include "tm.h"
#include "Collector.h"

static char cmrcsid[] =
      "$Id$";


  #include "htrsim.h"


 typedef short CURR;
 typedef unsigned short RES3;
 typedef unsigned short RES2;
 typedef unsigned short CAP2;
 typedef short TEMP;
 typedef unsigned short UINT;
/* Skeleton console_functions section */
/* Photon and resource manager probably don't mix */

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

void tminitfunc(void) {
}

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
#define LCMMN 16
#define ROLLOVER_MFC 0
#define SYNCHVAL 0xABB4
#define INVSYNCH 0
#define NROWMAJF 16
#define MFSECNUM 4
#define MFSECDEN 1
#define SECDRIFT 90
#define TM_DATA_TYPE TMTYPE_DATA_T3

/* Skeleton data_defs section */
#include "DG_data.h"

/* Some temporary defs until everything is in the right place */
#ifndef TS_MFC_LIMIT
  #define TS_MFC_LIMIT 32767
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


/* Skeleton main_program section */
// ### Make collector a #define and create a subclass
// ### for subbus that overrides the event(dq_event_stop) and
// ### Collect_row() methods
int main(int argc, char **argv) {
  // oui_init_options(argc, argv);
  collector col;
  col.init();
  col.receive("HtrData", &HtrData, sizeof(HtrData), 0);
  col.operate();
  return 0;
}


/* Skeleton pre_other section */
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
  if ( NROWMINF == 1 || MINF_ROW == NROWMINF-1 ) {
    /* Last row of minor frame: Synch Calculations */
    if ( INVSYNCH && collector::majf_row == NROWMAJF-1)
      Synch = ~SYNCHVAL;
    else
      Synch = SYNCHVAL;
  }
  
  /* appropriate collection function */
  home = (union home_row *) row[last];
  efuncs[collector::majf_row]();
  incmod(collector::majf_row, NROWMAJF);
  rowlets += TRD;
  commit_rows(MFCtr, MINF_ROW, 1);
  if ( NROWMINF == 1 || MINF_ROW == NROWMINF-1 ) {
    MFCtr = next_minor_frame;
    next_minor_frame++;
    MINF_ROW_ZERO;
  } else MINF_ROW_INC;
}


/* Skeleton COL_send section */
#ifdef COLRECVIMPLEMENTED
#include <stddef.h>

static void read_col_send( void *dest, size_t length,
					struct colmsg *cmsg, pid_t sent_tid ) {
  size_t trulen = min( length, cmsg->u.data.size );
  if ( trulen <= MAX_COLMSG_SIZE-5 ) {
	memcpy( dest, cmsg->u.data.data, trulen );
  } else {
	Readmsg( sent_tid,
	  offsetof( struct colmsg, u.data.data ), dest, trulen );
  }
}
#endif

/* Skeleton DG_other_decls section */
#ifdef COLRECVIMPLEMENTED
  struct colmsg *cmsg;
/* Skeleton DG_other_cases section */
	case COL_SEND:
	  cmsg = (struct colmsg *)msg_ptr;
	  switch (cmsg->id) {
		case COL_SEND_INIT:
		  if (stricmp(cmsg->u.name, "HtrData") == 0) {
			cmsg->u.data.id = 1;
			cmsg->u.data.size = sizeof(HtrData);
		  } else return reply_byte(sent_tid,DAS_UNKN);
		  cmsg->type = DAS_OK;
		  Reply(sent_tid, cmsg, offsetof(struct colmsg, u.data.data));
		  return 0;
		case COL_SEND_SEND:
		  switch (cmsg->u.data.id) {
			case 1:
			  read_col_send(&HtrData, sizeof(HtrData), cmsg, sent_tid );
			  break;
			default: return reply_byte(sent_tid, DAS_UNKN);
		  }
		  break;
		case COL_SEND_RESET: break;
		default: return reply_byte(sent_tid,DAS_UNKN);
	  }
	  return reply_byte(sent_tid, DAS_OK);
/* Skeleton "rest of the file" section */
#endif

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

/* Skeleton End of File */

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
