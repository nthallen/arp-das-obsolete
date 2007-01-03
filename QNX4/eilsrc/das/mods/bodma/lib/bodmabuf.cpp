/* BODMABUF.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BODMABUF.CPP 4-Jul-94,23:37:26,`THOMAS'
     Removed the use of templates for DMA_buffer class, always assume type is
     16 bit integer since this is what come out of DMA from a Mike anyways.
     The module is split into bodmabuf.h and bodmabuf.cpp
1:1 BODMABUF.CPP 6-Jul-94,15:31:14,`JEROMEC' No change
1:2 BODMABUF.CPP 17-Jul-94,18:08:50,`THOMAS'
     Modified to work in a DLL. Also added a version of copy_from_dma that
     works in an interrupt service routine and does not use int86x(), also it
     returns an error code instead of using exception handling because
     exception handling is not safe for interrupt routines.
1:3 BODMABUF.CPP 2-Aug-94,18:33:32,`THOMAS'
     Replace the use of int86x by inline assembly using the asm keyword. The
     int86x routine call DosGlobalLock which is not reentrant and causes Windows
     to crash after a while in the kernel.
     Also add detection of VDS services.
1:4 BODMABUF.CPP 1-Jun-95,17:40:18,`TBUIJS'
     Changed the name of the object BoDmabuf, added a test case in the code
     so that the testing code is contained with the object. The test routines
     are disabled with #if 0, #endif block.
1:5 BODMABUF.CPP 11-Jul-95,11:50:28,`TBUIJS'
     Added function headers to document the functions and changed the ASM
     sequences so as not to use 32 bit registers and require tasm to compile, the
     new version is also more efficient.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include "bodmabuf.h"
extern void bigmemmove (char *dest, char *src, long length);

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDmabuf::BoDmabuf
File:   BODMABUF.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDmabuf::BoDmabuf (unsigned long npts) throw (BoError)

		npts           Number of 16 bit points

		Exception      BoError (DMA_ERROR) if the buffer allocation fails

Description
		This constructor allocates a DMA buffer with npts 16 bit points.
		It requires the presence of a VDS server.
 #$%!........................................................................*/


#ifndef __QNX__
BoDmabuf::BoDmabuf (unsigned long npts) throw (BoError) :
	dds_off (FP_OFF (&dds)), dds_seg (FP_SEG (&dds))

{
	//check if VDS is present (bit 5 on = vds present)
	if (!(*(unsigned char *)MK_FP(0x40, 0x7b) & 0x20))
		{
		throw (BoError (DMA_ERROR));
		}

	// allocate DMA buffer
	memset (&dds, 0, sizeof (DDS));
	dds.region_size = npts * sizeof (short);

	asm push es
	_DI = dds_off;
	_ES = dds_seg;
	asm
		{
		mov ax, 8107h
		xor dx, dx
		int 4bh
		pop es
		jb  err
		}
#else
BoDmabuf::BoDmabuf (unsigned long npts) throw (BoError)
{
  long i;

  i = npts * sizeof(short);
  if ((qnx_segment_raw_alloc(i,&q_dma)) == -1) goto err;

  if ( (dmafd = shm_open("Physical",O_RDWR,0)) == -1) goto err;
  if ( (virt_addr = (char *)mmap(0,i,PROT_READ | PROT_WRITE, MAP_SHARED,dmafd
			       ,q_dma.addr)) == (void *)(-1))
	goto err;

#endif
  return;
err:
  throw (BoError (DMA_ERROR));
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDmabuf::~BoDmabuf
File:   BODMABUF.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDmabuf::~BoDmabuf ()

Description
		The DMA buffer is freed by this destructor.
 #$%!........................................................................*/

BoDmabuf::~BoDmabuf ()
{
int e;
#ifndef __QNX__
	// free DMA buffer
	asm push es
	_DI = dds_off;
	_ES = dds_seg;
	asm
		{
		mov ax, 8108h
		xor dx, dx
		int 4bh
		pop es
		}
#else
e = errno;
	qnx_segment_raw_free(&q_dma);
e = errno;
	shm_unlink("Physical");
e = errno;
#endif
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDmabuf::copy_to_dma
File:   BODMABUF.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDmabuf::copy_to_dma (const BoMemory<short> & block,
							   unsigned long din, unsigned long sin,
							   unsigned long npts) throw (BoError)

		block          Memory block
		din            Destination index (in DMA buffer)
		sin            Source index (in memory block)
		npts           Number of 16 bit points to copy

		Exception	   BoError (INVALID_INDEX) if the copy would go beyond
		                                       the edge of either buffer
					   BoError (DMA_ERROR) if The copy fails

Description
		This function copies a block of 16 bit points from a memory buffer
		to a DMA buffer.
 #$%!........................................................................*/

void BoDmabuf::copy_to_dma (const BoMemory<short> & block,
	unsigned long din, unsigned long sin, unsigned long npts) throw (BoError)
{
	if (block.size () < sin+npts)
		{
		throw (BoError (INVALID_INDEX));
		}

	if (copy_to_dma_isr (block, din, sin, npts) != NO_ERROR)
		{
		throw (BoError (DMA_ERROR));
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDmabuf::copy_to_dma_isr
File:   BODMABUF.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        short BoDmabuf::copy_to_dma_isr (const BoMemory<short> & block,
								   unsigned long din, unsigned long sin,
								   unsigned long npts)

		block          Memory block
		din            Destination index (in DMA buffer)
		sin            Source index (in memory block)
		npts           Number of 16 bit points to copy

		returns        NO_ERROR if all went well
		               DMA_ERROR if the copy failled

Description
		This function copies a block of 16 bit points from a memory buffer
		to a DMA buffer. This function uses a return code instead of
		of exception handling to return error codes so that it can be used
		in interrupt routines.
 #$%!........................................................................*/

short BoDmabuf::copy_to_dma_isr (const BoMemory<short> & block,
	unsigned long din, unsigned long sin, unsigned long npts)
{
	// copy into DMA buffer
	din *= sizeof (short);

#ifndef __QNX__
	dds.selector = FP_SEG (block.p + sin);
	dds.offset = FP_OFF (block.p + sin);
	dds.region_size = npts * sizeof(short);

	asm push es
	_DI = dds_off;
	_ES = dds_seg;
	// set up for copy with VDS, cx = low din, bx = high din
	_AX = 0x8109;
	_BX = ((unsigned short *)&din)[1];
	_CX	= (unsigned short)din;
	asm
		{
		xor dx, dx
		int 4bh
		pop es
		jb  err
		}
#else
	long i;
	i = npts * sizeof(short);
	/* assume din and sin are 0 for me now eil */
/*	movedata(FP_SEG (block.p + sin),FP_OFF (block.p + sin),
		FP_SEG((char *) q_dma + din), FP_OFF((char *)q_dma + din),
			 i);
*/
/*	bigmemmove((char *)MK_FP(seg1,0), (char *)block.p, i);*/

	bigmemmove(virt_addr, (char *)block.p, i);
#endif

	return (NO_ERROR);
err:
	return (DMA_ERROR);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDmabuf::copy_from_dma
File:   BODMABUF.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDmabuf::copy_from_dma (const BoMemory<short> & block,
							   unsigned long din, unsigned long sin,
							   unsigned long npts) throw (BoError)

		block          Memory block
		din            Destination index (in memory block)
		sin            Source index (in DMA buffer)
		npts           Number of 16 bit points to copy

		Exception	   BoError (INVALID_INDEX) if the copy would go beyond
		                                       the edge of either buffer
					   BoError (DMA_ERROR) if The copy fails

Description
		This function copies a block of 16 bit points from a DMA buffer
		to a memory block.
 #$%!........................................................................*/

void BoDmabuf::copy_from_dma (BoMemory<short> & block,
	unsigned long din, unsigned long sin, unsigned long npts) throw (BoError)
{
	if (block.size () < din+npts)
		{
		throw (BoError (INVALID_INDEX));
		}

	if (copy_from_dma_isr (block, din, sin, npts) != NO_ERROR)
		{
		throw (BoError (DMA_ERROR));
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDmabuf::copy_from_dma_isr
File:   BODMABUF.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoDmabuf::copy_from_dma_isr (const BoMemory<short> & block,
							   unsigned long din, unsigned long sin,
							   unsigned long npts)

		block          Memory block
		din            Destination index (in memory block)
		sin            Source index (in DMA buffer)
		npts           Number of 16 bit points to copy

		Exception	   BoError (INVALID_INDEX) if the copy would go beyond
		                                       the edge of either buffer
					   BoError (DMA_ERROR) if The copy fails

Description
		This function copies a block of 16 bit points from a DMA buffer
		to a memory block. This function uses a return code instead of
		of exception handling to return error codes so that it can be used
		in interrupt routines.
 #$%!........................................................................*/

short BoDmabuf::copy_from_dma_isr (BoMemory<short> & block,
					unsigned long din, unsigned long sin, unsigned long npts)
{
	// copy out of DMA buffer
	sin *= sizeof (short);

#ifndef __QNX__
	dds.selector = FP_SEG (block.p + din);
	dds.offset = FP_OFF (block.p + din);
	dds.region_size = npts * sizeof (short);

	asm push es
	_DI = dds_off;
	_ES = dds_seg;
	// set up for copy with VDS, cx = low sin, bx = high sin
	_AX = 0x810a;
	_BX = ((unsigned short *)&sin)[1];
	_CX = (unsigned short)sin;
	asm
		{
		xor	dx, dx
		int 4bh
		pop es
		jb	err
		}
#else
	long i;
	i = npts * sizeof(short);
/*	movedata(FP_SEG((char *)q_dma + sin), FP_OFF((char *)q_dma + sin),
		FP_SEG (block.p + din),FP_OFF (block.p + din),
			npts * sizeof(short));
*/
/*	bigmemmove((char *)block.p, (char *)MK_FP(seg1,0), i);*/

	bigmemmove((char *)block.p, virt_addr, i);
#endif
	return (NO_ERROR);
err:
	return (DMA_ERROR);
}

// ******************************************************************
// test code.
// ******************************************************************

#ifdef DEBUG
// DOS and QNX test code
#include <iostream.h>

#include "bo_error.h"
#include "bomemory.h"
#include "bodmabuf.h"

void prt (short num, BoMemory<short> &buf, long bufsize)
{
	long i;

	cout << num << "-buf = ";
	for (i = 0; i < 4; i++)
		{
		cout <<	buf.p[i] << ",";
		}
	for (i = bufsize-4; i < bufsize; i++)
		{
		cout <<	buf.p[i] << ",";
		}
	cout << "\n";
}

main (int argc, char *argv[])
{
	if (argc != 2)
		{
		cout << "Specify buffer size in 16 bit words\n";
		return (-1);
		}

	long bufsize = atol (argv[1]);

	try
		{
		BoMemory<short> buf (bufsize, 1);
		BoMemory<short> buf2 (bufsize, 1);
		BoDmabuf dma_buf (bufsize);
		long i;

		for (i = 0; i < bufsize; i++)
			{
			buf.p[i] = (short)(i%32768L);
			}

		prt (1, buf, bufsize);

		dma_buf.copy_to_dma (buf, 0, 0, bufsize);

		prt (2, buf, bufsize);

		buf.init (0);

		prt (3, buf, bufsize);

		dma_buf.copy_from_dma (buf, 0, 0, bufsize);

		prt (4, buf, bufsize);

		dma_buf.copy_from_dma (buf, 0, 3, bufsize-3);

		prt (5, buf, bufsize);

		dma_buf.copy_from_dma (buf, 2, 0, bufsize-2);

		prt (6, buf, bufsize);

		dma_buf.copy_from_dma (buf, 5, 7, bufsize-7);

		prt (7, buf, bufsize);

		dma_buf.copy_from_dma (buf, 0, 0, bufsize);

		dma_buf.copy_to_dma (buf, 2, 0, bufsize-2);
		dma_buf.copy_from_dma (buf2, 0, 0, bufsize);

		prt (8, buf2, bufsize);

		dma_buf.copy_to_dma (buf, 0, 1, bufsize-1);
		dma_buf.copy_from_dma (buf2, 0, 0, bufsize);

		prt (9, buf2, bufsize);

		dma_buf.copy_to_dma (buf, 6, 8, bufsize-8);
		dma_buf.copy_from_dma (buf2, 0, 0, bufsize);

		prt (10, buf2, bufsize);
		}

	catch (BoError err)
		{
		cout << "Error " << err.number () << "\n";
		return (-1);
		}

	return (0);
}
#endif

