/* BOALLOC.C */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BOALLOC.C 10-Sep-93,11:50:58,`THOMAS' File creation
1:1 BOALLOC.C 29-Mar-94,11:26:00,`JEAN'
     Added the standard file header and TLIB marker.
1:2 BOALLOC.C 14-Apr-94,12:49:14,`JEAN' Change TLIB marker
1:3 BOALLOC.c 14-Apr-94,15:33:22,`JEAN' Added missing # in front of endif
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <malloc.h>
#include <string.h>
#ifndef __QNX__
  #include <dos.h>
#endif
#include <assert.h>
#include "useful.h"


#ifndef BO_DEBUG
/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_ALLOC
File:   BOALLOC.C
Author: Claude Lafond
Date:   May 1, 1990

Synopsis
        #include "useful.h"

        void HPTR *bo_alloc (long length);

        length          Number of bytes to be allocated

        return          NULL on error (defined in "stddef.h")
                        else pointer to the allocated memory block

Description
        Allocates a block of ram from the far heap.  Used for inter
        compiler portability.

Cautions
        Use bo_free () to release the block. A check for a NULL pointer
        should always be performed, this indicates insufficient memory.

See also
        bo_free()
#$%!.........................................................................*/

void HPTR *bo_alloc (long length)
{
#ifndef DOSX286
#ifdef __TURBOC__
	void *ptr;
#endif
#else
	extern unsigned _maxfarrealloc;

	_maxfarrealloc = (unsigned)((length/(1024L*1024L)+1) * (1024/64));
#endif

#ifdef __WATCOMC__
	return (halloc (length, 1));
#endif
#ifdef _MSC_VER
	return (halloc (length, 1));
#endif
#ifdef __TURBOC__
#ifdef DOSX286
	return ((void HPTR *)farmalloc (length));
#else
	ptr = farmalloc (length+12);
	if (ptr == NULL) return(NULL);
	assert(FP_OFF(ptr)==4);
	return ((void HPTR *)MK_FP(FP_SEG(ptr)+1, 0));
#endif
#endif
}

/*#$%!*************************************************************************
					 COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_REALLOC
File:   BOALLOC.C
Author: Michel Baillargeon
Date:   August 25, 1992

Synopsis
		#include "useful.h"

		void HPTR *bo_realloc (void HPTR *block, long size);

		block           Block to resize
		size            New size for block

		return          NULL on error (defined in "stddef.h")
						else pointer to the reallocated memory block

Description
		Changes the size of a block previously allocated with bo_alloc()
		without loosing the contents of the original block. If the size is
		reduced the end of the block will be lost but not the beginning.
		If there is not enough memory this function will return a NULL.

Cautions
		Use bo_free() to release the block. You should always check for a
		NULL pointer
See also
		bo_free(), bo_alloc(), bo_srealloc()
#$%!.........................................................................*/
#if 0
void HPTR *bo_realloc(void HPTR *block, long size)
{
#ifndef DOSX286
	void HPTR *p;
#else
	extern unsigned _maxfarrealloc;

	_maxfarrealloc = (unsigned)((size/(1024L*1024L)+1) * (1024/64));
#endif

#if (defined __WATCOMC__ || defined _MSC_VER)
	if ((p=bo_alloc(size))==NULL)
		return(NULL);
	if (block != NULL)
		{
		bigmemmove(p, block, size);
		bo_free(block);
		}
	return(p);
#endif

#ifdef __TURBOC__
#ifdef DOSX286
	return ((void HPTR *)farrealloc ((void *)block, size));
#else
/*	p = farrealloc ((void *)((char HPTR *)block-12), size+12);*/ /*gab 8-sep-93*/
	p = farrealloc( (void *)((char HPTR *)MK_FP(FP_SEG(block)-1,4)), size+12);
	if (p == NULL) return(NULL);
	assert(FP_OFF(p)==4);
	return ((void HPTR *)MK_FP(FP_SEG(p)+1, 0));
#endif
#endif
}
#endif
/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_FREE
File:   BOALLOC.C
Author: Claude Lafond
Date:   May 1, 1990

Synopsis
        #include "useful.h"

        void bo_free (void HPTR *ptr);

        ptr             Memory block to be released

Description
        Frees a block allocated with bo_alloc ()

See also
        bo_alloc()
#$%!.........................................................................*/

void bo_free (void HPTR *ptr)
{
#ifdef __WATCOMC__
	(void)hfree (ptr);
#endif
#ifdef _MSC_VER
	(void)hfree (ptr);
#endif
#ifdef __TURBOC__
#ifdef DOSX286
	(void)farfree ((void *)ptr);
#else
	(void)farfree ((void *)(MK_FP(FP_SEG(ptr)-1,4)));	/*gab 8-sep-93*/
#endif
#endif
}
/*#$%!*************************************************************************
					 COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_SALLOC
File:   BOALLOC.C
Author: Claude Lafond
Date:   May 1, 1990

Synopsis
		#include "useful.h"

		void *bo_salloc (unsigned short length);

		length          Number of bytes to be allocated

		return          NULL on error (defined in "stddef.h")
						else pointer to the allocated memory block

Description
		Allocates a block of ram from the heap.  Used for portability.

Cautions
		Use bo_sfree () to release the block. You should always check for a
		NULL pointer

See also
		bo_sfree()
#$%!.........................................................................*/

void *bo_salloc (unsigned short length)
{

	return (malloc (length));
}
/*#$%!*************************************************************************
					 COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_STRDUP
File:   BOALLOCD.C
Author: Thomas Buijs
Date:   November 12, 1991

Synopsis
		#include "useful.h"

		char *bo_strdup (char *s);

		s               String to copy

		return          NULL on error (defined in "stddef.h")
						else pointer to the allocated memory block

Description
		Makes a copy of the string s in a dynamically allocated block and
		returns a pointer to the new block.

Cautions
		Use bo_sfree () to release the block. You should always check for a
		NULL pointer
See also
		bo_sfree(), bo_salloc()
#$%!i.........................................................................*/

char *bo_strdup(char *s)
{
	return(strdup(s));
}

/*#$%!*************************************************************************
					 COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_SREALLOC
File:   BOALLOCD.C
Author: Thomas Buijs
Date:   November 12, 1991

Synopsis
		#include "useful.h"

		void *bo_srealloc (void *block, unsigned short size);

		block           Block to resize
		size            New size for block

		return          NULL on error (defined in "stddef.h")
						else pointer to the reallocated memory block

Description
		Changes the size of a block previously allocated with bo_sallocd()
		without loosing the contents of the original block. If the size is
		reduced the end of the block will be lost but not the beginning.
		If there is not enough memory this function will return a NULL.

Cautions
		Use bo_sfree () to release the block. You should always check for a
		NULL pointer
See also
		bo_sfree(), bo_salloc()
#$%!i.........................................................................*/

void *bo_srealloc(void *block, unsigned short size)
{
	return(realloc(block, size));
}
/*#$%!*************************************************************************
					 COPYRIGHT (C) BOMEM INC, 1990

Name:   BO_SFREE
File:   BOALLOC.C
Author: Claude Lafond
Date:   May 1, 1990

Synopsis
		#include "useful.h"

		void bo_sfree (void *ptr);

		ptr             Memory block to be released

Description
		Frees a block allocated with bo_salloc ()

See also
		bo_salloc()
#$%!.........................................................................*/

void bo_sfree (void *ptr)
{

	(void)free (ptr);
}

#endif
