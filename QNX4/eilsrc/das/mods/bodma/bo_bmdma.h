/* BO_BMDMA.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BO_BMDMA.H 10-Jul-95,13:58:38,`TBUIJS'
     Definition for the bus master DMA class.
1:1 BO_BMDMA.H 12-Dec-95,12:06:28,`TBUIJS'
     Change the programming of the seq36 to use 20% of the available bandwidth
     instead of 80% as was always done before. This changed resolved a lock up
     problem that occured during acquisition under Windows 95.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __BO_BMDMA_H
#define __BO_BMDMA_H

#include <conio.h>

#include "bodmabuf.h"
#include "bo_error.h"

// Bus master DMA interface
class BoBmdma
	{
		short dma_channel;
		short dma_ioadr;
		unsigned long size;				// size of DMA transfer in bytes
		const unsigned char mask_reg;	// DMA mask register
		const unsigned char mode_reg;	// DMA mode register
	public:
		BoBmdma (short channel, short ioadr) throw (BoError);
		~BoBmdma ();
		void transfer (BoDmabuf &buf, unsigned long npts);
		void stop ();
		unsigned long status (BoDmabuf &buf);
		unsigned short mike_stat ();
	};

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBmdma::transfer
File:   BO_BMDMA.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBmdma::transfer (BoDmabuf &buf, unsigned long npts);

		buf				DMA buffer to transfer into
		npts            Number of points to transfer

Description
		This function initiates a bus master DMA transfer in burst mode
		using 80% bus capacity into the DMA buffer pointed to by buf. The
		number of points to transfer is in npts.
 #$%!........................................................................*/

inline void BoBmdma::transfer (BoDmabuf &buf, unsigned long npts)
{
	unsigned short page = (unsigned short)(buf.physical_address () >> 16);
	unsigned short offset = (unsigned short)(buf.physical_address ());

	size = npts;

	// 80% use 6,7; 60% use 14, 15; 40% use 22, 23; 20% use 30, 31

	(void) outpw (dma_ioadr+4, 6);     // stop DMA, Mike data, burst mode
	(void) outp (dma_ioadr+4, 0);      // reset FIFO
	(void) outpw (dma_ioadr, offset);  // DMA buffer address
	(void) outp (dma_ioadr, page);
	(void) outpw (dma_ioadr+2, (unsigned short)(-size)); // 24 bit count
	(void) outp (dma_ioadr+2, (-size >> 16));
	(void) outpw (dma_ioadr+4, 7);	   // start DMA transfer
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBmdma::stop
File:   BO_BMDMA.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBmdma::stop ();

Description
		Stop DMA transfer.
 #$%!........................................................................*/

inline void BoBmdma::stop ()
{
	(void) outpw (dma_ioadr+4, 0);     // stop all DMA
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBmdma::status
File:   BO_BMDMA.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        unsigned long BoBmdma::status (BoDmabuf &buf);

		buf				DMA buffer that is being transfered into

		returns         The number of points remaining in the current
 		                transfer

Description
		This function reports on how many points are left in the current
		DMA transfer that is writing into DMA buffer buf.
 #$%!........................................................................*/

inline unsigned long BoBmdma::status (BoDmabuf &buf)
{
	long count = ((inpw (dma_ioadr) + ((long)inp (dma_ioadr)<<16)) -
											buf.physical_address ()) / 2;
	return (size - count);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBmdma::mike_stat
File:   BO_BMDMA.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        unsigned BoBmdma::mike_stat ();

		returns         The raw Michelson status word

Description
		This function returns the Michelson status word for the most
		recently completed scan.
 #$%!........................................................................*/

inline unsigned short BoBmdma::mike_stat ()
{
	return (inpw (dma_ioadr+6));
}

#endif
