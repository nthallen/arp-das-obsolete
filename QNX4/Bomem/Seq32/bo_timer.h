/* BO_TIMER.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/* This software is the property of Bomem and should be considered and      */
/* treated as proprietary information.  Refer to the "Source Code License   */
/* Agreement"                                                               */
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BO_TIMER.H 5-Oct-92,12:31:10,`THOMAS' File creation
1:1 BO_TIMER.H 15-Apr-94,9:29:22,`JEAN' Added the new TLIB header
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

/* Time related functions */

#ifndef	BOMEM_TIMER
#define	BOMEM_TIMER

#ifndef	BOMEM_USEFUL
	#include "USEFUL.H"
#endif

#ifndef	CLK_TCK
	#include <time.h>
#endif
#ifndef	ULONG_MAX
	#include <limits.h>
#endif

#define TIMER_FOREVER		ULONG_MAX
#define TIMER_NOWAIT		0L

#define MAX_TIMER 9			/* For timer_start() and timer_get() */

/* TIMER_C2.C */
short timer_start(short index_timer, long time_delay);
short timer_get(short index_timer);

/* TIMER_C.C */
void bo_timer_start (time_t *timer, long time_delay);
short bo_timer_get (time_t *timer);

/* TIMERDOS.C */
void chrono_start (long *tick);
float chrono_get (long *tick);
short bo_wait_fun (float delay, short (*waitfun)(float rem_time));
void bo_wait(float delay);

/* TIME2.C */
time_t bo_get_time_t(short year, char month, char day, char hour, char min,
					 char sec);

/* BO_TIME.C */
void bo_gettime(short *hour, short *minut, short *sec, short *hund);
void bo_getdate(short *year, short *month, short *day);

#endif
