/* BO_TIME.C */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BO_TIME.C 16-Jun-93,18:10:30,`THOMAS' File creation
1:1 BO_TIME.C 29-Mar-94,11:26:00,`JEAN'
     Added the standard file header and TLIB marker.
1:2 BO_TIME.C 14-Apr-94,12:49:16,`JEAN' Change TLIB marker
1:3 BO_TIME.c 14-Apr-94,15:33:24,`JEAN' Added missing # in front of endif
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stddef.h>		/* For NULL defn */
#include <time.h>
#ifndef __QNX__
  #include <dos.h>
#endif

#include "useful.h"
#include "bo_timer.h"

/*#$%!i*************************************************************************
					 COPYRIGHT (C) BOMEM INC, 1991

Name:   BO_GETTIME
File:   BO_TIME.C
Author: Thomas Buijs
Date:   January 8, 1991

Synopsis
		#include "timer.h"
		void bo_gettime(short *hour, short *minut, short *sec, short *hund);

		hour     pointer to current hour
		minut    pointer to current minute
		sec      pointer to current second
		hund     pointer to current one hundredth of a second

Description
		This function returns the current system time. It is similar
		to the C library function gettime() or _dos_gettime().
 #$%!i........................................................................*/

void bo_gettime(short *hour, short *minut, short *sec, short *hund)
{
#ifdef __TURBOC__
	struct time t;
#elif defined(__QNX__)
	struct tm *t;
	time_t tod;
#else
	struct dostime_t t;
#endif

#ifdef __TURBOC__
	gettime(&t);
	*hour=t.ti_hour;
	*minut=t.ti_min;
	*sec=t.ti_sec;
	*hund=t.ti_hund;
#elif defined(__QNX__)
	tod = time( NULL );
	t = localtime(&tod);
	*hour = t->tm_hour;
	*minut = t->tm_min;
	*sec = t->tm_sec;
	*hund = 0;
#else
	_dos_gettime(&t);
	*hour=t.hour;
	*minut=t.minute;
	*sec=t.second;
	*hund=t.hsecond;
#endif
}

/*#$%!i*************************************************************************
					 COPYRIGHT (C) BOMEM INC, 1991

Name:   BO_GETDATE
File:   BO_TIME.C
Author: Thomas Buijs
Date:   January 8, 1991

Synopsis
		#include "timer.h"
		void bo_getdate(short *year, short *month, short *day);

		year     pointer to current year
		month    pointer to current month
		day      pointer to current day

Description
		This function returns the current system date. It is similar
		to the C library function getdate() or _dos_getdate().
 #$%!i........................................................................*/

void bo_getdate(short *year, short *month, short *day)
{
#ifdef __TURBOC__
	struct date d;
#elif defined(__QNX__)
	struct tm *t;
	time_t tod;
#else
	struct dosdate_t d;
#endif

#ifdef __TURBOC__
	getdate(&d);
	*year=d.da_year;
	*day=d.da_day;
	*month=d.da_mon;
#elif defined(__QNX__)
	tod = time( NULL );
	t = localtime(&tod);
	*year = t->tm_year + 1900;
	*day = t->tm_mday;
	*month = t->tm_mon;
#else
	_dos_getdate(&d);
	*year=d.year;
	*day=d.day;
	*month=d.month;
#endif
}
