/* TIMERDOS.C */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 TIMERDOS.C 14-Apr-93,9:23:20,`THOMAS' File creation
1:1 TIMERDOS.C 29-Mar-94,11:26:12,`JEAN'
     Added the standard file header and TLIB marker.
1:2 TIMERDOS.C 14-Apr-94,12:49:28,`JEAN' Change TLIB marker
1:3 TIMERDOS.c 14-Apr-94,15:33:36,`JEAN'
     Added missing # in front of endif
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stddef.h>		/* For NULL defn */
#include <time.h>
#ifndef __QNX__
  #include <dos.h>
#endif
#include "useful.h"
#include "bo_timer.h"

static long chrono_time;

static long get_time_100(void);

/*#$%!************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   CHRONO_START
File:   TIMERDOS.C
Author: Michel Baillargeon
Date:   May 1, 1990

Synopsis
        #include "timer.h"

        void chrono_start (long *time_buf);

        time_buf        Pointer to starting time value,
                        if NULL use internal value (for single chrono)

Description
        This function starts a chronometer to measure elapsed time. It can
        be called multiple times with different pointers for multiple
        simultaneous chronometers.  This should not be used to measure
        events of a duration longer than 24 hours since it wraps around
        after 24 hours.  The time precision is dictated by the PC
        hardware (tick counter @ 18.2 Hz) and allows events to be measured
        with a precision of about 55 ms.

See also
        chrono_get()

 #$%!.........................................................................*/

void chrono_start (long *time_buf)
{
	if (time_buf == NULL)
		chrono_time = get_time_100();
	else
		*time_buf = get_time_100();
}

/*#$%!************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   CHRONO_GET
File:   TIMERDOS.C
Author: Michel Baillargeon
Date:   May 1, 1990

Synopsis
        #include "timer.h"

        float chrono_get (long *time_buf);

        time_buf        Pointer to starting time value,
                        if NULL use internal value (for single chrono)

        Returns         Elapsed time in seconds

Description
        This function gets the elapsed time since chrono_start() was called
        with a time pointer. This should not be used to measure events of a
        duration longer than 24 hours since it wraps around after 24 hours.
        The time precision is dictated by the PC hardware
        (tick counter @ 18.2 Hz) and allows events to be measured with a
        precision of about 55 ms.

See also
        chrono_start()

 #$%!.........................................................................*/

float chrono_get (long *time_buf)
{
long now, elapsed;

	now = get_time_100();
	if (time_buf == NULL)
		elapsed = now - chrono_time;
	else
		elapsed = now - *time_buf;

	if (elapsed < 0)
		elapsed += 8640000L;		/* Manage day change */

	return(elapsed / 100.0f);
}

/*#$%!************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_WAIT_FUN
File:   TIMERDOS.C
Author: Francois Gallichand
Date:   March 12, 1991

Synopsis
        #include "timer.h"

        short bo_wait_fun (float delay, short (*waitfun)(float rem_time));

        delay           Delay in seconds (maximum 24 hours = 86399.9 seconds)
        waitfun         User function, if NULL no function is called.
                        The function is called repeatedly while waiting for
                        the end of the delay. It is called with the
                        following parameter:
            rem_time    Remaining time in seconds until the exhaustion of
                        the delay.

            Returns     = 0:    
                            NO_ERROR          Continue
                       != 0:
                                              Abort delay and return
                                              error code

        Returns         = 0:
                            NO_ERROR          Everything is ok
                        < 0:
                            ERROR

Description
        This function should be used instead of a typical delay loop to slow
        down a process. The delay can be up to 24 hours with a precicion of
        approximately .06 sec. A function to be called repeatedly until
        the delay is over can be specified; the remaining time in seconds
        before "delay" is over is passed to the function so that it can
        return immediately if the delay is almost elapsed.

 #$%!.........................................................................*/

short bo_wait_fun (float delay, short (*waitfun)(float rem_time))
{
float rem_time;
long start_time;
short errcode;

							/* Check for no delay */
	if (delay <= 0.0f) return(NO_ERROR);
							/* Check for maximum delay */
	if (delay > 86399.9f) delay = 86399.9f;

							/* Start chrono */
	chrono_start (&start_time);

							/* Loop until delay is over */
	do	{
		rem_time = delay - chrono_get (&start_time);

		if (rem_time < 0.0f)
			 rem_time = 0.0f;

		if((*waitfun) != NULL)
			if ((errcode=(*waitfun)(rem_time))!=NO_ERROR)
				return(errcode);

		} while (rem_time >= (0.5f/CLK_TCK));
	return (NO_ERROR);
}

/*#$%!************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_WAIT
File:   TIMERDOS.C
Author: Michel Baillargeon
Date:   May 1, 1990

Synopsis
        #include "timer.h"

        void bo_wait (float delay);

        delay           Delay in seconds

Description
        This function should be used instead of a typical delay loop to
        slow down a process.  Be careful, not to specify a very long delay
        since it can't be interrupted.
        The resolution of the delay is about .06 seconds with PC hardware.

See also
        bo_wait_fun()

 #$%!.........................................................................*/

void bo_wait (float delay)
{

	(void) bo_wait_fun (delay, NULL);
}



static long get_time_100(void)
{
short hour, min, sec, hund;

							/* Get initial time */
	bo_gettime(&hour, &min, &sec, &hund);
	return((hour*360000L) + (min*6000L) + (sec*100) + hund);

}
