/* LDB_32.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						    SOURCE CODE										*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 LDB_32.CPP 9-Dec-94,14:16:14,`CLAUDE' File creation
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif
/*#$%!*************************************************************************
                     COPYRIGHT (c) BOMEM INC, 1994

Name:   Load .BIN to SEQ32
File:   LDB_32.CPP
Author: Claude Lafond
Date:   Nov 14, 1994

Synopsis
		short load_seq32 (char reset, short seq32_base, char *filename);

		reset		TRUE if SEQ32 must be reset (always the case normally)
		seq32_base	SEQ32 i/o base address
		filename	Microcode file name

		Returns		NO_EROR i everything ok
        
Description
		Load filename ucode in SEQ32 at seq32_base i/o address after a reset if
		reset is TRUE.
 #$%!........................................................................*/
#include <stdio.h>

#include "bo_timer.h"
#include "filegen.h"
#include "seq32_pc.h"

short load_seq32 (char reset, short seq32_base, char *filename)
{
long ucode_len;
long HPTR *ucode;
FILE *infile;
short i;

	/* Open ucode file length and determine its length */
	if ((infile=fopen (filename, "rb")) == NULL) return (ERROR);

	fseek (infile, 0, SEEK_END);
	ucode_len = ftell (infile);
	fseek (infile, 0, SEEK_SET);	/* Point to the beginning of the file */

	if ((ucode = (long HPTR *)bo_alloc(ucode_len)) == NULL)
		{
		fclose (infile);			/* Unable to allocate input buffer */
		return (ERROR);
		}
	if (flread (ucode, ucode_len, infile) != NO_ERROR)
		{
		bo_free(ucode);
		fclose(infile);
		return (ERROR);
		}
	fclose (infile);

	if (reset)
		{
		seq32_reset (seq32_base, 0);
		bo_wait (0.1);
		}

	for (i=0; i < 2; i++)
		{
		if (NO_ERROR == seq32_bootstrap (ucode, ucode_len/4l))
			{
			break;
			}
		}
	bo_free (ucode);

	return (i == 2) ? ERROR  :  NO_ERROR;
}

