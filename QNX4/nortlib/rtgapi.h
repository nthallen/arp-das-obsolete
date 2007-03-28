/* rtgapi.h defines the realtime API to rtg
 * $Log$
 * Revision 1.3  2007/03/28 19:27:33  nort
 * rtg_sequence
 *
 * Revision 1.1  1994/12/12  16:23:16  nort
 * Initial revision
 *
 */
#ifndef _RTGAPI_H_INCLUDED
#define _RTGAPI_H_INCLUDED

#include <sys/types.h>
#include "company.h"

typedef struct {
  unsigned short id;
  unsigned short ver;
  char module[2];
  union {
	char name[1];
	struct {
	  short int channel_id;
	  double X;
	  double dXorY;
	  short int n_pts;
	  float Y[1];
	} pt;
  } u;
} rtg_msg_t;
/* RTG_MSG_ID goes in the id field */
#define RTG_MSG_ID 'rt'

/* RTG_VERSION goes in the ver field */
#define RTG_VERSION 0

/* RTG_MOD_CDB is the "realtime" module (moduel[0]) */
#define RTG_MOD_CDB 'c'
/* These are the CDB module message subtypes (module[1]) */
#define RTG_CDB_CREATE 'c'
#define RTG_CDB_REPORT 'r'
#define RTG_CDB_SEQUENCE 's'

/* This is the name registered with QNX so other procs can find us */
#define RTG_NAME COMPANY "/rtg"

typedef struct {
  char *name;
  pid_t pid;
  rtg_msg_t msg;
  unsigned char deleted:1;
  unsigned char initialized:1;
} rtg_t;

rtg_t * rtg_init(char *name);
int rtg_report(rtg_t *rtg, double X, double Y);
int rtg_sequence(rtg_t *rtg, double X0, double dX, int n_pts, float *Y);
void rtgext_init( void );

int rtg_increment( int, int );
#define rtg_increment( x, y ) ( ++x >= y ? x=0 : x )

#endif
