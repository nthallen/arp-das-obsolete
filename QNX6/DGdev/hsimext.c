/* extmain.skel include file for extraction
 * Revision 1.1  2008/07/03 15:11:07  ntallen
 * Copied from QNX4 version V1R9
 */
/* <%headers% */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "oui.h"
#include "DC.h"

static char emrcsid[] =
      "$Id$";

/* %headers%> */

  #include "htrsim.h"


 typedef short CURR;
 typedef unsigned short RES3;
 typedef unsigned short RES2;
 typedef unsigned short CAP2;
 typedef short TEMP;
 typedef unsigned short UINT;

/* <%console_functions% */

/* %console_functions%> */

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
char *US_5_0_d_0( unsigned short int x) {
  static char obuf[6];
  int iov;

  obuf[5] = '\0';
  obuf[4] = (x % 10) + '0';
  iov = x/10;
  if (iov == 0) goto space3;
  obuf[3] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) goto space2;
  obuf[2] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) goto space1;
  obuf[1] = (iov % 10) + '0';
  iov /= 10;
  if (iov == 0) goto space0;
  obuf[0] = (iov % 10) + '0';
  goto nospace;
  space3: obuf[3] = ' ';
  space2: obuf[2] = ' ';
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}
char *SS_6_2_f_c( short int x) {
  static char obuf[7];
  int neg;

  if (x < -9999) return("******");
  if (x < 0) { neg = 1; x = -x; }
  else neg = 0;
  obuf[6] = '\0';
  obuf[5] = (x % 10) + '0';
  x /= 10;
  obuf[4] = (x % 10) + '0';
  x /= 10;
  obuf[3] = '.';
  obuf[2] = (x % 10) + '0';
  x /= 10;
  if (x == 0) {
    if (neg) { obuf[1] = '-'; goto space0; }
    else goto space1;
  }
  obuf[1] = (x % 10) + '0';
  x /= 10;
  if (x == 0) {
    if (neg) { obuf[0] = '-'; goto nospace; }
    else goto space0;
  }
  obuf[0] = (x % 10) + '0';
  goto nospace;
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}
/* _CVT_0() int tcvt TEMP -> TEMP */
char *_CVT_0(TEMP x) {
  if ( x < -9999 )
    return "******";
  return SS_6_2_f_c( x );
}

static void nullfunc(void);
static void BF1_0(void) {
  printf("%s %s\n",US_5_0_d_0(MFCtr),_CVT_0(home->U0.Thtr));
}

static void (*efuncs[16])() = {
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0,
  BF1_0
};

void tminitfunc() {}

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

/* <%data_defs% */
#ifndef EXTRACTION_INIT
  #define EXTRACTION_INIT
#endif
#ifndef OPT_EXT_INIT
  #define OPT_EXT_INIT
#endif
#ifndef EXTRACTION_TERM
  #define EXTRACTION_TERM
#endif
#ifndef TMINITFUNC
  #define TMINITFUNC
#endif

/* %data_defs%> */

/* <%main_program% */

int main(int argc, char **argv) {
  // oui_init_options(argc, argv);
  data_client DC(4096, 1, 0);
  DC.operate();
  return 0;
}
/* %main_program%> */

/* <%everything_else% */

/* The main thing we need to do is reset our row counter on TM START */

#if (NROWMINF == 1)
  #define MINF_ROW 0
  #define MINF_ROW_ZERO
  #define MINF_ROW_INC
#else
  #define MINF_ROW data_client::minf_row
  #define MINF_ROW_ZERO data_client::minf_row = 0
  #define MINF_ROW_INC ++data_client::minf_row
#endif

#define incmod(x,y) if (x==((y)-1)) x = 0; else x++

#if TM_DATA_TYPE == TMTYPE_DATA_T3
void data_client::process_data() {
  tm_data_t3_t *data = &msg->body.data3;
  unsigned char *raw = &data->data[0];
  int n_rows = data->n_rows;
  home = (union home_row *) raw;
  MFCtr = data->mfctr;
  #ifdef IVFUNCS
    unsigned short delta = MFCtr - next_minor_frame;
    if (delta != 0) {
        // Because we are TMTYPE_DATA_T3, we know NROWMINF == 1
        if (delta > NROWMAJF) delta = NROWMAJF;
        while (delta-- > 0) {
          ivfuncs[majf_row]();
          incmod(majf_row, NROWMAJF);
        }
    }
  #endif
  majf_row = (((unsigned short)MFCtr) % NROWMAJF);

  for ( ; n_rows > 0; --n_rows, ++home ) {
    efuncs[majf_row]();
    incmod(majf_row, NROWMAJF);
    ++MFCtr;
  }
  next_minor_frame = MFCtr;
}
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

/* %everything_else%> */

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
