/* BO_BMDMA.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BO_BMDMA.CPP 7-Jul-95,9:39:48,`TBUIJS'
     This object is equivalent to the bo_pcdma object but interfaces to the BUS
     master DMA controller on the seq36 card.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif
#include <errno.h>
#include "bo_bmdma.h"

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBmdma::BoBmdma
File:   BO_BMDMA.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        BoBmdma::BoBmdma (short channel, short ioadr) throw (BoError);

		channel         Dma channel to use
		ioadr           IO address of the bus master card

		Exception       BoError (DMA_ERROR) if the card is not found, there
		                         			is no VDS server or the VDS
											server returns and error status
						BoError (INVALID_ARGUMENT) if one of the parameters
						                           is out of range
Description
		This constructor checks to see whether it can locate a DMA bus
		master card at the given io	address. It then sets up the DMA channel
		and resets all the hardware.

 #$%!........................................................................*/

BoBmdma::BoBmdma (short channel, short ioadr) throw (BoError) :
	mask_reg (0xd4), mode_reg (0xd6)
{
#ifndef __QNX__
	//check if VDS is present (bit 5 on = vds present)
	if (!(*(unsigned char *)MK_FP(0x40, 0x7b) & 0x20))
		{
		throw (BoError (DMA_ERROR));
		}
#endif

	REGS regs;

	dma_ioadr = ioadr;
	dma_channel = channel;

	// check card version numbers, if the card is not there this will fail
	if ((inpw (dma_ioadr+4) & 0xf000) != 0) // check card version number
		{
		throw (BoError (NO_HARDWARE));
		}
	if ((inp (dma_ioadr+3) & '\x78') != 0) // check card version number
		{
		throw (BoError (NO_HARDWARE));
		}

	// reset card
	outp (dma_ioadr+6, 0);
	outpw (dma_ioadr+4, 0);
	outp (dma_ioadr+4, 0);

	// program PC dma controller for cascade mode
	switch (channel)
		{
		case 5: // DMA channel 5
			outp (mode_reg, 0xc1);
			outp (mask_reg, 1);
			break;
		case 6: // DMA channel 6
			outp (mode_reg, 0xc2);
			outp (mask_reg, 2);
			break;
		case 7: // DMA channel 7
			outp (mode_reg, 0xc3);
			outp (mask_reg, 3);
			break;
		default:
			throw (BoError (INVALID_ARGUMENT));
		}

	// reset instrument (this operation must not be done immediatly after
	// the card reset because the card needs a small delay after it is reset
	// before it is ready to send commands to the michelson.
	outp (dma_ioadr+5, 0xc0);

#ifndef __QNX__
	// reserve DMA channel and disable address translation with VDS
	memset (&regs, 0, sizeof (REGS));
	regs.x.ax = 0x810b;
	regs.x.bx = (unsigned short) channel;
	//regs.x.dx = 0;

	(void) int86 (0x4b, &regs, &regs);
	if (regs.x.cflag)
		{
		throw (BoError (DMA_ERROR));
		}
#endif
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBmdma::BoBmdma
File:   BO_BMDMA.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        BoBmdma::~BoBmdma ()

Description
		The destructor stops any DMA in progress masks the DMA controller
		and releases the DMA channel back to the operating system.

 #$%!........................................................................*/

BoBmdma::~BoBmdma ()
{

int e;
e = errno;
	(void) outpw (dma_ioadr+4, 0); // stop DMA
e = errno;
	(void) outp (mask_reg, dma_channel); // mask channel
e = errno;
#ifndef __QNX__
	REGS regs;
	// free DMA channel and disable address translation with VDS
	memset (&regs, 0, sizeof (REGS));
	regs.x.ax = 0x810c;
	regs.x.bx = (unsigned short) dma_channel;
	//regs.x.dx = 0;

	(void) int86 (0x4b, &regs, &regs);
#endif
}

// ******************************************************************
// test code.
// ******************************************************************

#ifdef DEBUG
// DOS and QNX test code
#include <iostream.h>
#include <conio.h>

#include "bodmabuf.h"
#include "bo_bmdma.h"

main ()
{
	BoDmabuf dma (32767L);
/*	BoBmdma dmac (1); */
	BoBmdma dmac (5,310);
	BoMemory<short> buf (32767L);
 
	dmac.transfer (dma, 32767L);
/*	while (!kbhit ()) cout << dmac.status () << "\n";*/
	while (!kbhit ()) cout << dmac.status (dma) << "\n";
	getch ();
	dma.copy_from_dma (buf, 0, 0, 9);

	return 0;
}
#endif
