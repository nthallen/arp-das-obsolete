/* FLACCESS.C */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 FLACCESS.C 9-Apr-93,13:58:26,`THOMAS' File creation
1:1 FLACCESS.C 28-Jan-94,19:03:26,`THOMAS' added check for BC4 in
1:2 FLACCESS.C 28-Jan-94,19:20:08,`THOMAS' detect BC4
1:3 FLACCESS.C 29-Mar-94,11:26:04,`JEAN'
     Added the standard file header and TLIB marker.
1:4 FLACCESS.C 14-Apr-94,12:49:20,`JEAN' Change TLIB marker
1:5 FLACCESS.c 14-Apr-94,15:33:26,`JEAN'
     Added missing # in front of endif
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stdio.h>
#include <stdlib.h>
#ifdef __QNX__
  #include <i86.h>
  template<class T> T min(T a, T b) { return a<b?a:b; };
#else
  #include <dos.h>
#endif

#ifdef __TURBOC__
#ifdef __cplusplus
#if __BORLANDC__ != 0x452
#include  <stdtempl.h>
#endif
#endif
#endif

#include "useful.h"
#include "filegen.h"

static short bo_read(void HPTR *buffer, long length, FILE *file);
static short bo_write(void HPTR *buffer, long length, FILE *file);

/*#$%!************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   FLREAD
File:   FLACCESS,C
Author: Michel Baillargeon
Date:   May 1, 1990

Synopsis:
        #include "filegen.h"

        short flread (void HPTR *buffer, long length, FILE *file);

        buffer          Input buffer pointer
        length          Number of bytes to read
        file            File pointer

        Returns         NO_ERROR
                        ERROR

Description:
        This function is equivalent to fread() except that it works
        with buffer lengths larger than 32K.
 #$%!.........................................................................*/
/* TB 26-FEB-1992, modify for protected mode compatibility */

short flread (void HPTR *buffer, long length, FILE *file)
{
	unsigned short off;
	long block;
	char HPTR *buf;

	/* make a char * copy so that arithmetic works */
	buf = (char HPTR *)buffer;

	/* compute length of first block to segment boundary */
	off     = FP_OFF(buf);
	block   = min(65536L-off,length);
	length -= block;

	/* read first block and advance buffer pointer to next segment */
	if (bo_read(buf, block, file) != NO_ERROR) return(ERROR);
	
	/* read the rest of the file in 64K chunks and maybe 1 short chunk */
	while (length)
		{
		buf    += block;
		block   = min(length, 65536L);
		length -= block;

		if (bo_read(buf, block, file) != NO_ERROR) return(ERROR);
		}
	return(NO_ERROR);
}

/*****************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_READ
File:   FLACCESS.C
Author: Thomas Buijs
Date:   February 26, 1992

Synopsis:
        short bo_read (void HPTR *buffer, long length, FILE *file);

        buffer          Output buffer pointer
        length          Number of bytes to read
        file            File pointer

        Return          NO_ERROR
                        ERROR

Description:
		This is an internal function, it can read up to 64K but the block must
        reside in a single segment. This function is used by flread and should
        never be called directly.
 .............................................................................*/
static short bo_read(void HPTR *buffer, long length, FILE *file)
{
	word half, left;

	half = (word)(length/2);
	left = (word)(length%2);

	if (fread((void *)buffer, half, 1, file) != 1 ||
	    fread((void *)((char *)buffer+half), half, 1, file) != 1)
	  return(ERROR);
	if (left)
		{
		if (fread((void *)((char HPTR *)buffer+half*2), left, 1, file) != 1)
			return(ERROR);
		}
	return(NO_ERROR);
}

/*#$%!************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   FLWRITE
File:   FLACCESS.C
Author: Michel Baillargeon
Date:   May 1, 1990

Synopsis:
        #include "filegen.h"

        short flwrite (void HPTR *buffer, long length, FILE *file);

        buffer          Output buffer pointer
        length          Number of bytes to write
        file            File pointer

        Return          NO_ERROR
                        ERROR

Description:
        This function is equivalent to fwrite() except that it works
        with buffer lengths larger than 32K.
 #$%!.........................................................................*/

/* TB 26-FEB-1992, modify for protected mode compatibility */

short flwrite (void HPTR *buffer, long length, FILE *file)
{
	unsigned short off;
	long block;
	char HPTR *buf;

	/* make a char * copy so that arithmetic works */
	buf = (char HPTR *)buffer;

	/* compute length of first block to segment boundary */
	off     = FP_OFF(buf);
	block   = min(65536L-off,length);
	length -= block;

	/* write first block and advance buffer pointer to next segment */
	if (bo_write(buf, block, file) != NO_ERROR) return(ERROR);
	
	/* write the rest of the buffer in 64K chunks and maybe 1 short chunk */
	while (length)
		{
		buf    += block;
		block   = min(length, 65536L);
		length -= block;

		if (bo_write(buf, block, file) != NO_ERROR) return(ERROR);
		}
	return(NO_ERROR);
}

/*****************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_WRITE
File:   FLACCESS.C
Author: Thomas Buijs
Date:   February 26, 1992

Synopsis:
        short bo_write (void HPTR *buffer, long length, FILE *file);

        buffer          Output buffer pointer
        length          Number of bytes to write
        file            File pointer

        Return          NO_ERROR
                        ERROR

Description:
		This is an internal function, it can write up to 64K but the block must
        reside in a sigle segment. This function is used by flwrite and should
        never be called directly.
 .............................................................................*/
static short bo_write(void HPTR *buffer, long length, FILE *file)
{
	word half, left;

	half = (word)(length/2);
	left = (word)(length%2);

	if (fwrite((void *)buffer, half, 1, file) != 1 ||
	    fwrite((void *)((char *)buffer+half), half, 1, file) != 1)
	  return(ERROR);
	if (left)
		{
		if (fwrite((void *)((char HPTR *)buffer+half*2), left, 1, file) != 1)
			return(ERROR);
		}
	return(NO_ERROR);
}

