#ifndef ANALOGIC_H
#define ANALOGIC_H

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
} analogic_setup_t;

#define ANLG_OPT_A 1
#define ANLG_OPT_B 2
#define ANLG_OPT_C 4
#define ANLG_OPT_D 8
#define ANLG_OPT_FIT 0x10

typedef struct {
  unsigned short header;
  signed short type;
} analogic_msg_t;

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
	analogic_msg_t (ANLGC_HEADER, ANLGC_OK)
	analogic_rpt_t
	[ unsigned short Data[NReport][NChannels] (ANLGC_FMT_16IL)
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
} analogic_rpt_t;

#define ANLGC_FMT_DATAFMT 0x8000
#define ANLGC_FMT_16IL 0
#define ANLGC_FMT_FLOAT ANLGC_FMT_DATAFMT
#define ANLGC_FMT_FITTYPE 0x7FFF
#define ANLGC_FMT_FITNONE 0
#define ANLGC_FMT_FITLIN 0x10
#define ANLGC_FMT_FITLOG 0x20
#define fmt_float(x) ((x)&ANLGC_FMT_DATAFMT)
#define fmt_fittype(x) ((x)&ANLGC_FMT_FITTYPE)

/* A status message consists of:
	analogic_msg_t (ANLGC_HEADER, ANLGC_OK)
	analogic_status_t
*/
typedef struct {
  unsigned long index;
  unsigned short status;
} analogic_status_t;

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

Server_Def *cpci_init( char *name );
int cpci_setup( Server_Def *cpci, analogic_setup_t *setup );
int cpci_stop( Server_Def *cpci );
int cpci_quit( Server_Def *cpci );
int cpci_nolog( Server_Def *cpci );
int cpci_log( Server_Def *cpci );
int cpci_report( Server_Def *cpci, int raw, unsigned short index,
		analogic_rpt_t *rpt, void **data, float **fit, size_t size );
int cpci_status( Server_Def *cpci, analogic_status_t *status );

#endif

