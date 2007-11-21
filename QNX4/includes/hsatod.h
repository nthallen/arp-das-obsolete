#ifndef HSATOD_H
#define HSATOD_H

#include "cltsrvr.h"

typedef struct {
  unsigned long FSample;
  unsigned long NSample;
  unsigned long NReport;
  unsigned long NAvg;
  unsigned short NCoadd;
  unsigned short FTrigger;
  unsigned short Options;
  unsigned long TzSamples;
} hsatod_setup_t;

#define ANLG_OPT_A 1
#define ANLG_OPT_B 2
#define ANLG_OPT_C 4
#define ANLG_OPT_D 8
#define ANLG_OPT_FIT 0x10

typedef struct {
  unsigned short header;
  signed short type;
} hsatod_msg_t;

#define ANLGC_HEADER 'ag'
#define ANLGC_SETUP 'su'
#define ANLGC_STOP 'sp'
#define ANLGC_REPORT 'rp'
#define ANLGC_RAW  'rw'
#define ANLGC_STATUS 'st'
#define ANLGC_QUIT 'qu'
#define ANLGC_NOLOG 'nl'
#define ANLGC_LOG 'lg'

/* A report message consists of:
	hsatod_msg_t (ANLGC_HEADER, ANLGC_OK)
	hsatod_rpt_t
	  unsigned short Data[NReport][NChannels] (ANLGC_FMT_16IL)
		or
	  unsigned short Data[NChannels][NReport] (ANLGC_FMT_16NIL)
	    or
	  float Data[NChannels][NReport] (ANLGC_FMT_FLOAT) ]
	float FitData[NFit];
  Note the different order of the indices.
*/
typedef struct {
  unsigned short Format;
  unsigned short NChannels;
  unsigned short NReport;
  unsigned short NFit;
  unsigned short index;
  unsigned long findex;
} hsatod_rpt_t;

/* The Format code has changed a bit from Analogic.
   The upper 4 bits define the basic data layout
   and the lower 12 bits define the fit used if any.
   The three layouts supported are:
     16-bit unsigned integer interleaved
	 16-bit unsigned integer non-interleaved
	 32-bit float non-interleaved
   All raw data comes in one of the integer formats
   while binned data is reported as float.
   
   This format is compatible with the previous
   analogic usage, but data generated under
   this definition using ANLGC_FMT_16NIL
   would be mis-interpreted as an unknown
   fit type. Programs which interpret the
   Format (notably snapshot and lograw) should
   be recompiled and augmented to support the
   new ANLGC_FMT_16NIL. cpci14 should probably
   be recompiled.
*/
#define ANLGC_FMT_DATAFMT 0xF000
#define ANLGC_FMT_16IL 0
#define ANLGC_FMT_16NIL 0x1000
#define ANLGC_FMT_FLOAT 0x8000

#define ANLGC_FMT_FITTYPE 0x0FFF
#define ANLGC_FMT_FITNONE 0
#define ANLGC_FMT_FITSTD  1
#define ANLGC_FMT_FITLIN 0x10
#define ANLGC_FMT_FITLOG 0x20
#define fmt_float(x) ((x)&ANLGC_FMT_FLOAT)
#define fmt_fittype(x) ((x)&ANLGC_FMT_FITTYPE)

/* A status message consists of:
	hsatod_msg_t (ANLGC_HEADER, ANLGC_OK)
	hsatod_status_t
*/
typedef struct {
  unsigned long index;
  unsigned short status;
  unsigned short Vin1, Vin2;
} hsatod_status_t;

#define ANLGC_OK 0
#define ANLGC_E_SEND -1
#define ANLGC_E_MSG -2
#define ANLGC_E_UNKN -3
#define ANLGC_E_BUSY -4
#define ANLGC_E_SETUP -5
#define ANLGC_E_IDXOOR -6

#define ANLGC_S_UNINIT 0
#define ANLGC_S_READY 1
#define ANLGC_S_ARMED 2
#define ANLGC_S_TRIG 3
#define ANLGC_S_STOP 4
#define ANLGC_S_QUIT 5
#define ANLGC_S_QUITSIG 6

typedef unsigned long ULONG;
typedef unsigned short USHRT;

extern Server_Def *hsatod_init( char *name );
extern int hsatod_setup( Server_Def *cpci, hsatod_setup_t *setup );
extern int hsatod_stop( Server_Def *cpci );
extern int hsatod_quit( Server_Def *cpci );
extern int hsatod_nolog( Server_Def *cpci );
extern int hsatod_log( Server_Def *cpci );
extern int hsatod_report( Server_Def *cpci, int raw, unsigned short index,
		hsatod_rpt_t *rpt, void **data, float **fit, size_t size );
extern int hsatod_status( Server_Def *hsatod, hsatod_status_t *status );
#endif

