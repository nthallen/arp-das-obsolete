/* TIMER_C.C */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 TIMER_C.C 5-Oct-92,10:30:28,`THOMAS' File creation
1:1 TIMER_C.C 29-Mar-94,11:26:12,`JEAN'
     Added the standard file header and TLIB marker.
1:2 TIMER_C.C 14-Apr-94,12:49:28,`JEAN' Change TLIB marker
1:3 TIMER_C.c 14-Apr-94,15:33:36,`JEAN' Added missing # in front of endif
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <time.h>
#include <limits.h>
#include <stddef.h>

#include "useful.h"
#include "bo_timer.h"


/*#$%!i************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_TIMER_START
File:   TIMER_C.C
Author: Michel Baillargeon
Date:   May 1, 1990

Synopsis
        #include "timer.h"

        void bo_timer_start (time_t *timer, long time_delay);

        timer           Pointer to timer end time
        time_delay      > 0 delay in seconds 
                        = 0 -> No delay
                        < 0 wait forever
Description
        This function should be used to start a countdown timer.  Then the
        bo_timer_get() function should be regularly polled to check if the time 
        is elapsed.

See also
        bo_timer_get(), timer_get()
 #$%!i.........................................................................*/
void bo_timer_start (time_t *timer, long time_delay)
{
	if (time_delay < TIMER_NOWAIT)
		*timer = TIMER_FOREVER;						/* Wait forever */
	else
		*timer = time(NULL) + time_delay;
}

/*#$%!i************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_TIMER_GET
File:   TIMER_C.C
Author: Michel Baillargeon
Date:   May 1, 1990

Synopsis
        #include "timer.h"

        short bo_timer_get (time_t *timer);

        timer           Pointer to timer end time

        Returns         FALSE time not elapsed yet
                        TRUE  time elapsed

Description
        This function checks if the time interval previously setted using 
        bo_timer_start() is elapsed.  This works independantly for every timer
        number.

 See also
        bo_timer_start(), timer_get()
 #$%!i.........................................................................*/
short bo_timer_get (time_t *timer)
{
	if (*timer == TIMER_FOREVER) return(FALSE);		/* Wait forever */

	if (time(NULL) >= *timer)
		return(TRUE);  
	else
		return(FALSE);  
}
