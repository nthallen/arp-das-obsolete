/* BOFCLOSE.C */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BOFCLOSE.C 16-Jul-93,8:50:58,`THOMAS' File creation
1:1 BOFCLOSE.C 29-Mar-94,11:26:00,`JEAN'
     Added the standard file header and TLIB marker.
1:2 BOFCLOSE.C 14-Apr-94,12:49:16,`JEAN' Change TLIB marker
1:3 BOFCLOSE.c 14-Apr-94,15:33:22,`JEAN'
     Added missing # in front of endif
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif


/* Replacement for fclose(), close file even when error */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "useful.h"

/* Undefine macro in useful.h */
#ifdef fclose
	#undef fclose
#endif

/*#$%!i************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1993

Name:   BO_FCLOSE
File:   BOFCLOSE.C
Author: Michel Baillargeon
Date:   June 6, 1993

Synopsis
        #include "useful.h"

		int bo_fclose(FILE *stream);

		stream				File pointer returned by fopen()

		Returns:			0  : No_error
							EOF: Error closing file


Description
		Replacement for fclose() that closes the file even in cases
		where the stream buffer cannot be flushed.
 #$%!i.......................................................................*/

int bo_fclose(FILE *stream)
{
int err;

	if ( (err=fclose(stream)) != NO_ERROR)
		{
		/* An error has occured... */
		if (stream != NULL)
			{
			(void) setvbuf(stream, NULL, _IONBF, 0);	/* Free stream buffer */
			(void) fclose(stream);	 				/* Close again */
			}
		}

	return(err);
}

