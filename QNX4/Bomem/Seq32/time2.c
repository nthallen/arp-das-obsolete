/* TIME2.C */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 TIME2.C 27-May-92,11:52:04,`THOMAS' File creation
1:1 TIME2.C 29-Mar-94,11:26:12,`JEAN'
     Added the standard file header and TLIB marker.
1:2 TIME2.C 14-Apr-94,12:49:28,`JEAN' Change TLIB marker
1:3 TIME2.c 14-Apr-94,15:33:36,`JEAN' Added missing # in front of endif
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stddef.h>		/* For NULL defn */
#include <time.h>
#include "useful.h"
#include "bo_timer.h"

#if 0
static short month_t[13] = {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273,
							304, 334};
#endif

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1991

Name:   BO_GET_TIME_T
File:   TIME2.C
Author: Michel Baillargeon
Date:   Feb 13, 1991

Synopsis
        #include "timer.h"
        time_t bo_get_time_t(short year, char month, char day, char hour,
                             char min, char sec);
        year           year (should be between 1970 and 2099)
        month          month (from 1 to 12; 1->Jan, 12->Dec)
        day            day in month (1 to 31)
        hour           hour in day (0 to 23)
        min            minutes (0 to 59)
        sec            seconds (0 to 59)

        Returns        Number of seconds elapsed since 00:00:00
                       Jan 1, 1970 GMT

Description
        This function returns a "time_t" from a date.  It does somewhat the
        opposite of the C function localtime().  This function uses the
        timezone information but does not take care of daylight saving time.
 #$%!........................................................................*/
#if 0
time_t bo_get_time_t(short year, char month, char day, char hour, char min,
					 char sec)
{
short extra_days;		/* Take care of leap years */

	/* year is a leap year if: */
	/* ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) */

	/* Simplified expression valid up to 2100 (should be enough !!!!)*/
	extra_days = (year-1972) / 4;

	if(!((year-1972) % 4) && month < 3)
		extra_days--;

	/* Set timezone info from environment variables */
	tzset();

	return (timezone + sec + 60L*min + 3600L*hour +
			(day+extra_days+month_t[month])*86400L + (year-1970)*31536000L);
}
#endif 

time_t bo_get_time_t(short year, char month, char day, char hour, char min,
					 char sec)
{
struct tm tp;

	tp.tm_year = year - 1900;
	tp.tm_mon = month - 1;
	tp.tm_mday = day;
	tp.tm_hour = hour;
	tp.tm_min = min;
	tp.tm_sec = sec;
	tp.tm_isdst = -1;
	tp.tm_yday = -1;
	tp.tm_wday = -1;

	return(mktime(&tp));
}
