/* V_ADD.C */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 V_ADD.C 18-Mar-93,17:25:46,`THOMAS' File creation
1:1 V_ADD.C 29-Mar-94,11:26:14,`JEAN'
     Added the standard file header and TLIB marker.
1:2 V_ADD.C 14-Apr-94,12:49:32,`JEAN' Change TLIB marker
1:3 V_ADD.c 14-Apr-94,15:33:38,`JEAN' Added missing # in front of endif
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

Name:   V_ADD
File:   V_ADD.C
Author: Michel Baillargeon
Date:   May 1, 1990

Synopsis
        #include "vector.h"

        void v_add (float HPTR *vin1, float HPTR *vin2, float HPTR *vout,
                    long length);

        vin1            First input buffer pointer
        vin2            Second input buffer pointer
        vout            Output buffer pointer
        length          Number of points in the buffers

Description
        Adds two vectors element by element

See also
        v_sub, v_mult, v_div
 #$%!.........................................................................*/
/* TB May 1992, support for 286|dos extender & loop optimization */

void v_add (float HPTR *vin1, float HPTR *vin2, float HPTR *vout, long length)
{
	long i;

	for (i=length +1; --i;)
		*vout++ = *vin1++ + *vin2++;
}
