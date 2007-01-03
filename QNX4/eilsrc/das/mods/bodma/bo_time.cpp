/* BO_TIME.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BO_TIME.CPP 15-Jun-94,11:11:12,`THOMAS' File creation
1:1 BO_TIME.CPP 4-Jul-94,12:37:48,`THOMAS'
     Finished the implementation of the Date_time functions and fixed certain
     assumptions about the value of the month enum
1:2 BO_TIME.CPP 10-Aug-94,17:22:24,`THOMAS'
     Add definition for clock_tick and optimize wait function.
1:3 BO_TIME.CPP 16-Jun-95,16:21:58,`TBUIJS'
     Changed the name of the objects to BoDatetime and BoClock respectively to
     adhere to the new Bo... naming convention.
1:4 BO_TIME.CPP 6-Jul-95,20:19:40,`TBUIJS'
     Add function headers to functions.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <math.h>
#include <time.h>
#include <cstring.h>

extern volatile long tcks;
#include "bo_time.h"
#include "bo_error.h"

string BoDatetime::month_names[12] = {"January", "February", "March", "April",
									 "May", "June", "July", "August",
									 "September", "October", "November",
									 "December"};

string BoDatetime::day_names[7] = {"Sunday", "Monday", "Tuesday", "Wednesday",
								  "Thursday", "Friday", "Saturday"};

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::BoDatetime
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime::BoDatetime (double min, double sec);

		min            Time in minutes
		sec			   Time in seconds

Description
		Constructor for a time object from a time in minutes and seconds.
 #$%!........................................................................*/

BoDatetime::BoDatetime (double min, double sec)
{
	time = sec + 60*min;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::BoDatetime
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime::BoDatetime (double hrs, double min, double sec);

		hrs            Time in hours
		min            Time in minutes
		sec			   Time in seconds

Description
		Constructor for a time object from a time in hours, minutes
		and seconds.
 #$%!........................................................................*/

BoDatetime::BoDatetime (double hrs, double min, double sec)
{
	time = sec + 60*min + 3600*hrs;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::BoDatetime
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime::BoDatetime (double days, double min, double sec);

		days           Time in days
		hrs            Time in hours
		min            Time in minutes
		sec			   Time in seconds

Description
		Constructor for a time object from a time in days, hours, minutes
		and seconds.
 #$%!........................................................................*/

BoDatetime::BoDatetime (double days, double hrs, double min, double sec)
{
	time = sec + 60*min + 3600*hrs + 24*3600L*days;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::BoDatetime
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime::BoDatetime (short day, Month month, short year,
								short hrs, short min, double sec);

		day            Day in the month
		month          Month in the year
		year           calendar year
		hrs            Hour part of the time
		min            Minute part of the time
		sec            Seconds

		Exception      BoError (INVALID_ARGUMENT) if any of the parameters
		                                          are out of range for a
												  valid	date and time
Description
		Constructor for a time object from a date and time. 
 #$%!........................................................................*/

BoDatetime::BoDatetime (short day, Month month, short year, short hrs,
			short min, double sec)
{
	struct tm tp;
	time_t t;

	tp.tm_year = year - 1900;
	tp.tm_mon  = month.month - 1; // month is 0-11 for Ansi-C functions
	tp.tm_mday = day;
	tp.tm_hour = hrs;
	tp.tm_min  = min;
	tp.tm_sec  = 0;
	tp.tm_isdst= -1;
	tp.tm_yday = -1;
	tp.tm_wday = -1;

	if ((t = mktime (&tp)) == -1)
		{
		throw (BoError (INVALID_ARGUMENT));
		}
	time = t + sec;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::BoDatetime
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime::BoDatetime (short day, const string &month, short year,
								short hrs, short min, double sec);

		day            Day in the month
		month          Month in the year as a string
		year           calendar year
		hrs            Hour part of the time
		min            Minute part of the time
		sec            Seconds

		Exception      BoError (INVALID_ARGUMENT) if any of the parameters
		                                          are out of range for a
												  valid	date and time
Description
		Constructor for a time object from a date and time. The month is
 		passed as a string with the name of the month. The recognition
		of the month string is not case sensitive.
 #$%!........................................................................*/

BoDatetime::BoDatetime (short day, const string &month, short year, short hrs,
			short min, double sec)
{
	short i, flg;
	struct tm tp;

#ifndef __CSTRING
	flg = string::get_case_sensitive_flag ();
	string::set_case_sensitive (0); // not case sensitive
#endif

	for (i = 0; i < 12; i++)
		{
		if (month == month_names[i]) break;
		}

#ifndef __CSTRING
	string::set_case_sensitive (flg);
#endif

	if (i == 12)
		{
		throw (BoError (INVALID_ARGUMENT));
		}

	tp.tm_year = year - 1900;
	tp.tm_mon  = i;
	tp.tm_mday = day;
	tp.tm_hour = hrs;
	tp.tm_min  = min;
	tp.tm_sec  = 0;
	tp.tm_isdst= -1;
	tp.tm_yday = -1;
	tp.tm_wday = -1;

	time = mktime (&tp) + sec;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::operator +
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		BoDatetime operator + (const BoDatetime &d1, const BoDatetime &d2)

		d1             First time
		d2             Second time
		
		returns        The sum of two times

Description
		Operator to add two time together
 #$%!........................................................................*/

BoDatetime operator + (const BoDatetime &d1, const BoDatetime &d2)
{
	return (BoDatetime (d1.time + d2.time));
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::operator -
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		BoDatetime operator - (const BoDatetime &d1, const BoDatetime &d2)

		d1             First time
		d2             Second time
		
		returns        The difference of two times

Description
		Operator to subtract one time from another
 #$%!........................................................................*/

BoDatetime operator - (const BoDatetime &d1, const BoDatetime &d2)
{
	return (BoDatetime (d1.time - d2.time));
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::operator !=
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short operator != (const BoDatetime &d1, const BoDatetime &d2)

		d1             First time
		d2             Second time
		
		returns        0 if the times are equal, 1 if they are not

Description
		Operator to compare two time together and check for inequality
 #$%!........................................................................*/

short operator != (const BoDatetime &d1, const BoDatetime &d2)
{
	return (d1.time != d2.time);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::operator ==
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short operator == (const BoDatetime &d1, const BoDatetime &d2)

		d1             First time
		d2             Second time
		
		returns        1 if the times are equal, 0 if they are not

Description
		Operator to compare two time together and check for equality
 #$%!........................................................................*/

short operator == (const BoDatetime &d1, const BoDatetime &d2)
{
	return (d1.time == d2.time);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::operator >
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short operator > (const BoDatetime &d1, const BoDatetime &d2)

		d1             First time
		d2             Second time
		
		returns        0 d2 is later than d1, 1 if d2 is earlier than d1

Description
		Operator to compare two time together and check for rank
 #$%!........................................................................*/

short operator > (const BoDatetime &d1, const BoDatetime &d2)
{
	return (d1.time > d2.time);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::operator >=
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short operator >= (const BoDatetime &d1, const BoDatetime &d2)

		d1             First time
		d2             Second time
		
		returns        0 d2 is later than d1,
		               1 if d2 is earlier or equal to d1

Description
		Operator to compare two time together and check for rank
 #$%!........................................................................*/

short operator >= (const BoDatetime &d1, const BoDatetime &d2)
{
	return (d1.time >= d2.time);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::operator <
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short operator < (const BoDatetime &d1, const BoDatetime &d2)

		d1             First time
		d2             Second time
		
		returns        1 d2 is later than d1, 0 if d2 is earlier than d1

Description
		Operator to compare two time together and check for rank
 #$%!........................................................................*/

short operator < (const BoDatetime &d1, const BoDatetime &d2)
{
	return (d1.time < d2.time);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::operator <=
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short operator <= (const BoDatetime &d1, const BoDatetime &d2)

		d1             First time
		d2             Second time
		
		returns        1 d2 is later or equal to d1,
		               0 if d2 is earlier than d1

Description
		Operator to compare two time together and check for rank
 #$%!........................................................................*/

short operator <= (const BoDatetime &d1, const BoDatetime &d2)
{
	return (d1.time <= d2.time);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::total_seconds
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		double BoDatetime::total_seconds () const

		returns        The time in seconds, if the object held a data then
		               it is the time in seconds from January 1970 in the
					   same manner as the encoded by the C run time.

Description
		This function returns the number of seconds in the BoDatetime object
 #$%!........................................................................*/

double BoDatetime::total_seconds () const
{
	return (time);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::hms_hours
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		long BoDatetime::hms_hours () const

		returns        The time in hours assuming that the time will be
                       in hours, minutes and seconds

Description
		This function returns the hour part of the time in hours,
		minutes and seconds in the BoDatetime object
 #$%!........................................................................*/

long BoDatetime::hms_hours () const
{
	return (long (time/3600));
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::hms_minutes
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short BoDatetime::hms_minutes () const

		returns        The time in minutes assuming that the time will be
                       in hours, minutes and seconds

Description
		This function returns the minutes part of the time in hours,
		minutes and seconds in the BoDatetime object
 #$%!........................................................................*/

//lint -e790
short BoDatetime::hms_minutes () const
{
	return (short (time/60 - hms_hours () * 60));
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::hms_seconds
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		double BoDatetime::hms_seconds () const

		returns        The time in seconds assuming that the time will be
                       in hours, minutes and seconds

Description
		This function returns the seconds part of the time in hours,
		minutes and seconds in the BoDatetime object
 #$%!........................................................................*/

double BoDatetime::hms_seconds () const
{
	return (time - hms_hours () * 3600 - hms_minutes () * 60);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::dhms_days
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		long BoDatetime::dhms_days () const

		returns        The time in days assuming that the time will be
                       in days, hours, minutes and seconds

Description
		This function returns the day part of the time in days, hours,
		minutes and seconds in the BoDatetime object
 #$%!........................................................................*/

long BoDatetime::dhms_days () const
{
	return (long (time/3600/24));
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::dhms_hours
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short BoDatetime::dhms_hours () const

		returns        The time in hours assuming that the time will be
                       in days, hours, minutes and seconds

Description
		This function returns the hours part of the time in days, hours,
		minutes and seconds in the BoDatetime object
 #$%!........................................................................*/

short BoDatetime::dhms_hours () const
{
	return (short (time/3600 - dhms_days () * 24));
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::dhms_minutes
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short BoDatetime::dhms_minutes () const

		returns        The time in minutes assuming that the time will be
                       in days, hours, minutes and seconds

Description
		This function returns the minutes part of the time in days, hours,
		minutes and seconds in the BoDatetime object
 #$%!........................................................................*/

short BoDatetime::dhms_minutes () const
{
	return (short (time/60 - dhms_days () * 24 * 60 - dhms_hours () * 60));
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::dhms_seconds
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		double BoDatetime::dhms_seconds () const

		returns        The time in seconds assuming that the time will be
                       in days, hours, minutes and seconds

Description
		This function returns the seconds part of the time in days, hours,
		minutes and seconds in the BoDatetime object
 #$%!........................................................................*/

double BoDatetime::dhms_seconds () const
{
	return (time - dhms_days () * 3600 * 24 - dhms_hours () * 3600 -
			dhms_minutes () * 60);
}
//lint +e790

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_day
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short BoDatetime::date_day () const

		returns        The day in the month.

Description
		This function returns the day in the month of the date
		in the BoDatetime object
 #$%!........................................................................*/

short BoDatetime::date_day () const
{
	time_t t = (time_t) time;

	return (localtime (&t) -> tm_mday);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_weekday
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		char BoDatetime::date_weekday () const

		returns        The day in the week.

Description
		This function returns the day in the week of the date
		in the BoDatetime object
 #$%!........................................................................*/

char BoDatetime::date_weekday () const
{
	time_t t = (time_t) time;

	return (Day (char (localtime (&t) -> tm_wday)).day);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_dayname
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		string BoDatetime::date_dayname () const

		returns        The day in the week as a string.

Description
		This function returns the name of the day in the week of the date
		in the BoDatetime object as a text string.
 #$%!........................................................................*/

string BoDatetime::date_dayname () const
{
	return (day_names [date_weekday ()]);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_yearday
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short BoDatetime::date_yearday () const

		returns        The day in the year.

Description
		This function returns the day in the year of the date
		in the BoDatetime object
 #$%!........................................................................*/

short BoDatetime::date_yearday () const
{
	time_t t = (time_t) time;

	return (localtime (&t) -> tm_yday);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_month
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		char BoDatetime::date_month () const

		returns        The month.

Description
		This function returns the month of the date
		in the BoDatetime object
 #$%!........................................................................*/

char BoDatetime::date_month () const
{
	time_t t = (time_t) time;

	return (Month (char (localtime (&t) -> tm_mon + 1)).month);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_monthname
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		string BoDatetime::date_monthname () const

		returns        The month name.

Description
		This function returns the month of the date
		in the BoDatetime object as a string.
 #$%!........................................................................*/

string BoDatetime::date_monthname () const
{
	time_t t = (time_t) time;

	return (month_names [localtime (&t) -> tm_mon]);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_year
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short BoDatetime::date_year () const

		returns        The year.

Description
		This function returns the year of the date
		in the BoDatetime object
 #$%!........................................................................*/

short BoDatetime::date_year () const
{
	time_t t = (time_t) time;

	return (localtime (&t) -> tm_year + 1900);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_hour24
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short BoDatetime::date_hour24 () const

		returns        The hour on a 24 hour clock.

Description
		This function returns the hour of the date and time 
		in the BoDatetime object
 #$%!........................................................................*/

short BoDatetime::date_hour24 () const
{
	time_t t = (time_t) time;

	return (localtime (&t) -> tm_hour);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_hour
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short BoDatetime::date_hour () const

		returns        The hour on a 12 hour clock.

Description
		This function returns the hour of the date and time 
		in the BoDatetime object
 #$%!........................................................................*/

short BoDatetime::date_hour () const
{
	time_t t = (time_t) time;
	short h = localtime (&t) -> tm_hour;

	switch (h)
		{
		case 0:
		case 12:
			return (12);
		default:
			return (h % 12);
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_ampm
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		char BoDatetime::date_ampm () const

		returns        The ampm flag, 0 if morning, 1 if afternoon

Description
		This function returns a flag that is 0 in the morning and 1 in
		the afternoon based on the date and time in the BoDatetime object
 #$%!........................................................................*/

char BoDatetime::date_ampm () const
{
	time_t t = (time_t) time;
	short h = localtime (&t) -> tm_hour;
	
	return (h < 13 ? AM : PM);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_min
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		short BoDatetime::date_min () const

		returns        The minute.

Description
		This function returns the minute of the date and time 
		in the BoDatetime object
 #$%!........................................................................*/

short BoDatetime::date_min () const
{
	time_t t = (time_t) time;

	return (localtime (&t) -> tm_min);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::date_sec
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		double BoDatetime::date_sec () const

		returns        The seconds.

Description
		This function returns the seconds of the date and time 
		in the BoDatetime object
 #$%!........................................................................*/

double BoDatetime::date_sec () const
{
	time_t t = (time_t) time;

	return (localtime (&t) -> tm_sec + time - floor (time));
}

#ifdef __QNX__
const double BoClock::tick_rate = 18.2064819;	// ticks per second
const long BoClock::tick_wrap = 0x1800b0L;		// ticks in 24 hours
#else
const double BoClock::tick_rate = 20;		// ticks per second
const long BoClock::tick_wrap = 1728000;		// ticks in 24 hours
#endif

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::BoClock
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		BoClock::BoClock (const BoDatetime &time)

		time           the time to set the clock to.

		Exception      BoError (INVALID_ARGUMENT) if the time cannot be
		                    set in the real time clock.                        
Description
		This constructor sets the PC real time clock to the time specified.
 #$%!........................................................................*/

BoClock::BoClock (const BoDatetime &time)
{
	double sec, isec;
#ifdef __QNX__
	struct timespec my_time;
	sec = time.total_seconds ();
	isec = floor (sec);
	my_time.tv_sec = (long) isec;
	my_time.tv_nsec = (sec - isec) * 1000000000L;
	if (clock_settime(CLOCK_REALTIME, &my_time))
#else
	dosdate_t d;
	dostime_t t;

	d.year   = (unsigned int) time.date_year ();
	d.day    = (unsigned char) time.date_day ();
	d.month  = (unsigned char) time.date_month ();
	t.hour   = (unsigned char) time.date_hour24 ();
	t.minute = (unsigned char) time.date_min ();
	sec  = time.date_sec ();
	isec = floor (sec);
	t.second  = (unsigned char) isec;
	t.hsecond = (unsigned char) floor ((sec - isec) * 100);
	if (_dos_setdate (&d) || _dos_settime (&t)) 
#endif
		{
		throw (BoError (INVALID_ARGUMENT));
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::time
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		BoDatetime BoClock::time ()

		returns		   The current date and time in the real time clock
Description
		This function read the current date and time from the real time clock
		and returns them in a BoDatetime object. This clock is acurate to
		about 1/18 of a second.
 #$%!........................................................................*/

BoDatetime BoClock::time ()
{
	double sec;
#ifdef __QNX__
	struct timespec my_time;
	clock_gettime( CLOCK_REALTIME, &my_time);
	sec = my_time.tv_sec + (my_time.tv_nsec * 0.000000001);
	return (BoDatetime (sec));
#else
	dosdate_t d;
	dostime_t t;
	_dos_getdate (&d);
	_dos_gettime (&t);
	sec = t.second + t.hsecond/100.0;

	return (BoDatetime (d.day, d.month, d.year, t.hour, t.minute, sec));
#endif
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::wait
File:   BO_TIME.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
		void BoClock::wait (const BoDatetime &delay,
							short (*waitfnc)(const BoDatetime &remaining))

		delay          Time to wait for
		waitfnc        optional function to call in a tight loop while
		               waiting.
		  remaining    Time remaining in the wait period
		  returns      0 to continue the waiting, 1 to stop waiting
Description
		This functions waits for a specified amount of time, the second
		parameter is optional. If a second parameter is provided it must
		be a pointer to a function that is called repeatedly during the
		wait period. The function that is called can terminate the waiting
		at any time by returning a value other than zero.
		This function read the current date and time from the real time clock
		and returns them in a BoDatetime object. This clock is acurate to
		about 1/18 of a second.
 #$%!........................................................................*/

void BoClock::wait (const BoDatetime &delay,
				short (*waitfnc)(const BoDatetime &remaining))
{
	countdown (delay);
	if (waitfnc != NULL)
		{
		while (countdown ().total_seconds () > 0)
			{
			if (waitfnc (countdown ())) break;
			}
		}
	else
		{
		while (countdown ().total_seconds () > 0)
			{
			}
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::BoClock
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoClock::BoClock ();

Description
		Default constructor for BoClock, this constructor does nothing
 #$%!........................................................................*/
#ifdef __QNX__
	extern pid_t far my_ticks ();
	extern long tick_w;
#endif
BoClock::BoClock ()
{
#ifdef __QNX__
  tick_w = tick_wrap;
  my_ticks_iid = qnx_hint_attach((-1),&my_ticks, my_ds());
#endif
}

BoClock::~BoClock ()
{
#ifdef __QNX__
   qnx_hint_detach(my_ticks_iid);
#endif
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::get_tick
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        long BoClock::get_tick ();

		returns        The tick count in the BIOS data area

Description
		This function is used to read the tick count in the BIOS data
		area. It can be used in interrupt service routines to compute
		time delays when the other time functions cannot be used because
		they are not reentrant.
 #$%!........................................................................*/

long BoClock::get_tick ()
{
#ifndef __QNX__
	return (*(unsigned long *)MK_FP (0x40, 0x6c));
#else
	return(tcks);
#endif
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::countdown
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        void BoClock::coundown (const BoDatetime &delay);

		delay          Time to wait

Description
		Starts a countdown of given the amount of time in delay
 #$%!........................................................................*/

void BoClock::countdown (const BoDatetime &delay)
{
	end_time = time () + delay;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::countdown
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime BoClock::coundown ();

		returns        The remaining time in the countdown

Description
		Returns the remaining time in the current coundown. The
		countdown should ne initialized by calling countdown (delay)
		before calling countdown ().
 #$%!........................................................................*/

BoDatetime BoClock::countdown ()
{
	return (end_time - time ());
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::start_timer
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        void BoClock::start_timer ();

Description
		Start a timer.
 #$%!........................................................................*/

void BoClock::start_timer ()
{
	start_time = time ();
	_timer = 1;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::timer
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime BoClock::timer ();

		returns        The time the timer has been running

Description
		This function is used after start_timer to read the elapsed time.
		If stop_timer is called timer will return the elapsed time at the
		moment of the call to stop_timer.
 #$%!........................................................................*/

BoDatetime BoClock::timer ()
{
	return (_timer ? (time () - start_time) : stop_time);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::stop_timer
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime BoClock::stop_timer ();

		returns        The time the timer has been running

Description
		This function is used after start_timer to read the elapsed time and
		to stop the timer.
 #$%!........................................................................*/

BoDatetime BoClock::stop_timer ()
{
	stop_time = timer ();
	_timer = 0;
	return (stop_time);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::tick_add
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        long BoClock::tick_add (long current, long advance);

		current        Current tick count as read by get_tick
		advance        number of ticks to advance

		returns        The tick count that will be read by get_tick after
		               advance ticks have gone by

Description
		This function adds advance ticks to the tick count in current, and
		returns the tick count that will result after advance ticks have
		elapsed. This function keeps track of the 24 hour wrap around and
		will correctly estimate the resulting tick across a midnight wrap.
		The largest possible advance is 24 hours.
 #$%!........................................................................*/

long BoClock::tick_add (long current, long advance)
{
	return ((current + advance) % tick_wrap);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::tick_sub
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        long BoClock::tick_sub (long first, long second);

		first          First tick count
		second         Second tick count (must be latter than first)

		returns        The number of ticks between first and second.

Description
		This function computes the number of ticks that separate the first
		tick count from the second tick count even if midnight was crossed
		between the two tick counts.
 #$%!........................................................................*/

long BoClock::tick_sub (long first, long second)
{
	if (second >= first) // normal case no wrap around
		{
		return (second - first);
		}
	else // second is beyond wrap point
		{
		return (second+tick_wrap - first);
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoClock::tick_cmp
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        short BoClock::tick_cmp (long start, long current, long target);

		start          starting tick count
		current        current tick count
		target         target tick count

		returns        0 if the target has not yet been reached
		               1 if the target has been reached

Description
		This function is used to wait out a time delay. start is the tick
		count when the delay was started, current is the current tick count
		and target is the target tick count computed by tick_add.
		The function will return 0 if the target time has not yet passed
		and 1 if the target time has passed. This function works even
		across the midnight boundary.
 #$%!........................................................................*/

short BoClock::tick_cmp (long start, long current, long target)
{
	if (target >= start) // normal case no wrap around
		{
		// but still be careful, current might wrap	around
		return ((current >= target || current < start) ? 1 : 0);
		}
	else // target figure is beyond wrap point
		{
		return ((current >= target && current < start) ? 1 : 0);
		}
}


