/* extmain.skel include file for extraction
 * $Log$
 * Revision 1.2  2008/07/03 20:58:07  ntallen
 * In the process of testing.
 *
 * Revision 1.1  2008/07/03 15:11:07  ntallen
 * Copied from QNX4 version V1R9
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "oui.h"
#include "tm.h"
#include "globmsg.h"
#include "msg.h"

static char emrcsid[] =
      "$Id$";


  #include "htrsim.h"


 typedef short CURR;
 typedef unsigned short RES3;
 typedef unsigned short RES2;
 typedef unsigned short CAP2;
 typedef short TEMP;
 typedef unsigned short UINT;
#ifdef CONSOLE_INIT
  #error CONSOLE_INIT is obsolete
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


union {
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

static unsigned int next_minor_frame, majf_row;
#if (NROWMINF != 1)
  static unsigned int minf_row = 0;
#endif
#ifdef NEED_TIME_FUNCS
  static unsigned short CurMFC;
#endif


void main(int argc, char **argv) {
  oui_init_options(argc, argv);

  TMINITFUNC;
  BEGIN_MSG;
  EXTRACTION_INIT;
  DC_operate();
  EXTRACTION_TERM;
  DONE_MSG;
}


#define MKDCTV(x,y) ((x<<8)|y)
#define DCTV(x,y) MKDCTV(DCT_##x,DCV_##y)

/* The main thing we need to do is reset our row counter on TM START */
void DC_DASCmd(unsigned char type, unsigned char val) {
  switch (MKDCTV(type, val)) {
	case DCTV(TM,TM_START):
	  next_minor_frame = majf_row = 0;
	  #if (NROWMINF != 1)
		minf_row = 0;
	  #endif
	  break;
	case DCTV(TM,TM_END):
	case DCTV(QUIT,QUIT):
	default:
	  break;
  }
}

#define incmod(x,y) if (x==((y)-1)) x = 0; else x++

void DC_data(dbr_data_type *dr_data) {
  unsigned short nrows;

  for (nrows = dr_data->n_rows, home = (void *) &dr_data->data[0];
	   nrows > 0;
	   nrows--, home ++) {

	/* First check the minor frame counter if necessary */
	#if (NROWMINF != 1)
	if (minf_row == 0) {
	#endif
	  COPY_MFCtr /* Needs to be defined by TMC */
	  #ifdef NEED_TIME_FUNCS
		CurMFC = MFCtr;
	  #endif
	  {
		unsigned short delta;
		
		delta = MFCtr - next_minor_frame;
		if (delta != 0) {
		  #ifdef IVFUNCS
			if (delta > NROWMAJF/NROWMINF) delta = NROWMAJF;
			else delta *= NROWMINF;
			/* delta is now the number of rows skipped up to a maximum of
			  nrowmajf. While invalidating, I don't need to update
			  minf_row, since the final will be 0, same as before.
			  MFCtr is already set, and majf_row is updated.
			*/
			while (delta-- > 0)
			  ivfuncs[majf_row]();
		  #endif
		  majf_row = (((unsigned short)MFCtr) % (NROWMAJF/NROWMINF))
							  * NROWMINF;
		}
	  }
	  next_minor_frame = MFCtr+1;
	#if (NROWMINF != 1)
	}
	#endif
	
	/* Now process the row */
	efuncs[majf_row]();
	incmod(majf_row, NROWMAJF);
	#if (NROWMINF != 1)
	  incmod(minf_row, NROWMINF);
	#endif
  }
}

#ifdef __WATCOMC__
  #pragma off (unreferenced)
#endif

void DC_tstamp(tstamp_type *tstamp) { /* Nothing to do! */ }

void DC_other(unsigned char *msg_ptr, int sent_tid) {
  unsigned char rep_msg = DAS_UNKN;

  Reply(sent_tid, &rep_msg, 1);
}

#ifdef __WATCOMC__
  #pragma on (unreferenced)
#endif

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
