/* BODMABUF.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BODMABUF.H 20-Jun-94,18:40:46,`THOMAS' File creation
1:1 BODMABUF.H 4-Jul-94,23:37:26,`THOMAS'
     Removed the use of templates for DMA_buffer class, always assume type is
     16 bit integer since this is what come out of DMA from a Mike anyways.
     The module is split into bodmabuf.h and bodmabuf.cpp
1:2 BODMABUF.H 17-Jul-94,17:50:22,`THOMAS'
     Modified to work in a DLL. Also added a version of copy_from_dma that
     works in an interrupt service routine and does not use int86x(), also it
     returns an error code instead of using exception handling because
     exception handling is not safe for interrupt routines.
1:3 BODMABUF.H 2-Aug-94,16:48:32,`THOMAS'
     Add some comments, and eliminate variable in the class that were used for
     int86x which is no longer used (see bodmabuf.c). Also Interrupt versions
     of the copy from, to routines so that they can be used in interrupt service
     routines safely. The main point here is that there are no throw statements
     when the routines are used in interrupts because stack unwinding could be
     dangerous in an interrupt context!
1:4 BODMABUF.H 30-May-95,16:43:08,`TBUIJS'
     The name of the object is now BoDmabuf.
1:5 BODMABUF.H 6-Jul-95,20:19:32,`TBUIJS'
     Added headers to the inline functions.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __BODMABUF_H
#define __BODMABUF_H

#include <errno.h>
#include <dos.h>

#include "bo_error.h"
#include "bomemory.h"
#include <fcntl.h>
#include <sys/mman.h>


// Interface to obtain DMA buffers using VDS
class BoDmabuf
	{
#ifdef __QNX__
	  struct _seginfo q_dma;
//	  unsigned int seg1;
	  int dmafd;
	  char *virt_addr;
#else
		// DMA descriptor block for VDS services
		struct DDS
			{
			unsigned long region_size;
			unsigned long offset;
			unsigned short selector;
			unsigned short buffer_id;
			unsigned long physical_address;
			} dds;

		const unsigned short dds_off;
		const unsigned short dds_seg;
#endif

	public:
		BoDmabuf (unsigned long npts = 65536L) throw (BoError);
		~BoDmabuf ();
		unsigned long physical_address ();
		void copy_to_dma (const BoMemory<short> & block, unsigned long din,
						unsigned long sin, unsigned long npts)
						throw (BoError);
		short copy_to_dma_isr (const BoMemory<short> & block,
						unsigned long din, unsigned long sin,
						unsigned long npts);
		void copy_from_dma (BoMemory<short> & block, unsigned long din,
						unsigned long sin, unsigned long npts)
						throw (BoError);
		short copy_from_dma_isr (BoMemory<short> & block, unsigned long din,
								 unsigned long sin, unsigned long npts);
	};

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoDmabuf::physical_address ()
File:   BODMABUF.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        unsigned long BoDmabuf::physical_address ();

		returns				The physical address of the DMA buffer
Description
		This function returns the physical address of a DMA buffer.
 #$%!........................................................................*/

inline unsigned long BoDmabuf::physical_address ()
{
#ifndef __QNX__
	return (dds.physical_address);
#else
//	return(q_dma.addr);
//	return((unsigned long)dmaaddr);
	return((unsigned long)q_dma.addr);
#endif
}

#endif
