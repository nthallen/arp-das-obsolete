/* V_SCALE.C */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 V_SCALE.C 18-Mar-93,17:26:08,`THOMAS' File creation
1:1 V_SCALE.C 29-Mar-94,11:26:22,`JEAN'
     Added the standard file header and TLIB marker.
1:2 V_SCALE.C 14-Apr-94,12:49:40,`JEAN' Change TLIB marker
1:3 V_SCALE.c 14-Apr-94,15:33:46,`JEAN' Added missing # in front of endif
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stddef.h>
#include <math.h>
#include "useful.h"
#include "vector.h"

/*--------------------------------------------------------------*/
/* Data is treated by block of 8192 points (32 KBytes) to avoid */
/* using huge pointers internally since that would slows down   */
/* the computation.  This approach adds a little overhead for   */
/* tiny vectors but usually this is not a problem               */
/*--------------------------------------------------------------*/
/*-----Note---------------------------------------------------------*/
/* Be careful when using a far pointer not to decrement beyond the  */
/* beginning of the segment otherwise a wrap-around effect will     */
/* happen                                                           */
/*------------------------------------------------------------------*/

/*#$%!************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   V_SCALE
File:   V_SCALE.C
Author: Michel Baillargeon
Date:   May 1, 1990

Synopsis
        #include "vector.h"

        void v_scale (float HPTR *vin, float factor, float HPTR *vout,
                      long length);

        vin1            Input buffer pointer
        factor          Scale factor
        vout            Output buffer pointer
        length          Number of points in the buffers

Description
        Multiplies each point in a vector "vin" by a scale factor (a scalar)
        and stores the results in "vout"
See also
        v_offset, v_neg
 #$%!.........................................................................*/
/* TB May 1992, support for 286|dos extender & loop optimization */

void v_scale (float HPTR *vin, float factor, float HPTR *vout, long length)
{
	long i;

	for (i=length +1; --i;)
		*vout++ = *vin++ * factor;
}
