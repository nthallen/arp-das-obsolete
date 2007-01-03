/* BO_TIME.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BO_TIME.H 15-Jun-94,17:28:38,`THOMAS' File creation
1:1 BO_TIME.H 4-Jul-94,12:37:48,`THOMAS'
     Finished the implementation of the Date_time functions and fixed certain
     assumptions about the value of the month enum
1:2 BO_TIME.H 10-Aug-94,17:22:26,`THOMAS'
     Fix the logic in the timer routines and make the tick count an unsigned long
     instead of a signed long. The get_tick routine is now declared as static in
     the PC_clock class.
1:3 BO_TIME.H 16-Jun-95,15:10:32,`TBUIJS'
     Changed the name of the objects to BoDatetime and BoClock. Added routines to
     handle adding and comparing tick_counts taking the 24 hour wrap around into
     account.
1:4 BO_TIME.H 6-Jul-95,20:19:34,`TBUIJS'
     Add function headers to functions.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __BO_TIME_H
#define __BO_TIME_H

#include <dos.h>
#include <cstring.h>

#include "bo_error.h"

enum {JANUARY = 1, FEBRUARY, MARCH, APRIL, MAY, JUNE, JULY, AUGUST,
			SEPTEMBER, OCTOBER, NOVEMBER, DECEMBER};
enum {SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY};
enum {AM, PM};

// Class to hold an acurate representation of dates, times and time periods
class BoDatetime
	{
		static string month_names[12];
		static string day_names[7];
		double time;
	public:
		class Month
			{
			public:
				char month;
				Month (char _month = JANUARY);
			};

		class Day
		{
			public:
				char day;
				Day (char _day = SUNDAY);
			};

		class Ampm
			{
			public:
				char ampm;
				Ampm (char _ampm = AM);
			};

		BoDatetime (double sec = 0.0);
		BoDatetime (double min, double sec);
		BoDatetime (double hrs, double min, double sec);
		BoDatetime (double days, double hrs, double min, double sec);
		BoDatetime (short day, Month month, short year, short hrs=0,
				   short min=0, double sec=0.0);
		BoDatetime (short day, const string &month, short year, short hrs=0,
				   short min=0, double sec=0.0);

		friend BoDatetime operator + (const BoDatetime &, const BoDatetime &);
		friend BoDatetime operator - (const BoDatetime &, const BoDatetime &);

		friend short operator != (const BoDatetime &, const BoDatetime &);
		friend short operator == (const BoDatetime &, const BoDatetime &);

		friend short operator >  (const BoDatetime &, const BoDatetime &);
		friend short operator >= (const BoDatetime &, const BoDatetime &);
		friend short operator <  (const BoDatetime &, const BoDatetime &);
		friend short operator <= (const BoDatetime &, const BoDatetime &);

		double total_seconds () const;
		long   hms_hours () const;
		short  hms_minutes () const;
		double hms_seconds () const;
		long   dhms_days () const;
		short  dhms_hours () const;
		short  dhms_minutes () const;
		double dhms_seconds () const;
		short  date_day () const;
		char   date_weekday () const;
		string date_dayname () const;
		short  date_yearday () const;
		char   date_month () const;
		string date_monthname () const;
		short  date_year () const;
		short  date_hour24 () const;
		short  date_hour () const;
		char   date_ampm () const;
		short  date_min () const;
		double date_sec () const;
	};

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::Month::Month
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime::Month::Month (char _month);

		month           The month of the year (1=January...)

Description
		This constructor is used to validate month values.
 #$%!........................................................................*/

inline BoDatetime::Month::Month (char _month)
{
	if (_month < JANUARY || _month > DECEMBER)
 		{
		throw (BoError (INVALID_ARGUMENT));
		}
	Month::month = _month;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::Day::Day
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime::Day::Day (char _day);

		_day           The day of the week (0=Sunday...)

Description
		This constructor is used to validate day values.
 #$%!........................................................................*/

inline BoDatetime::Day::Day (char _day)
{
	if (_day < SUNDAY || _day > SATURDAY)
 		{
		throw (BoError (INVALID_ARGUMENT));
		}
	Day::day = _day;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::Ampm::Ampm
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime::Ampm::Ampm (char _ampm);

		_ampm          The afternoon flag, (0 for am or morning, 1 for pm)

Description
		This constructor is used to validate am pm values.
 #$%!........................................................................*/

inline BoDatetime::Ampm::Ampm (char _ampm)
{
	if (_ampm != AM && _ampm != PM)
 		{
		throw (BoError (INVALID_ARGUMENT));
		}
	Ampm::ampm = _ampm;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDatetime::BoDatetime
File:   BO_TIME.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDatetime::BoDatetime (double sec);

		sec            Time in seconds

Description
		Constructor for a time object from a time in seconds.
 #$%!........................................................................*/

inline BoDatetime::BoDatetime (double sec)
{
	time = sec;
}

//

// Object to interface to the PC real time clock and to manipulate
// clock ticks read from the BIOS data area
class BoClock
	{
		BoDatetime end_time;
		BoDatetime start_time, stop_time;
		char _timer;
		int my_ticks_iid;
	public:
		static const double tick_rate;
		static const long tick_wrap;
		BoClock ();
		~BoClock ();
		BoClock (const BoDatetime &time);
		static BoDatetime time ();
		void countdown (const BoDatetime &delay);
		BoDatetime countdown ();
		void start_timer ();
		BoDatetime timer ();
		BoDatetime stop_timer ();
		void wait (const BoDatetime &delay,
				   short (*waitfnc)(const BoDatetime &remaining) = NULL);
		static long get_tick ();
		static long tick_add (long current, long advance);
		static long tick_sub (long first, long second);
		static short tick_cmp (long start, long current, long target);
	};

#endif

