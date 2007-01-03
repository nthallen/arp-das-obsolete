/* BOFIFO.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BOFIFO.H 16-Jun-95,19:40:50,`TBUIJS'
     Definition for a new FIFO object that accepts large vectors as entries and
     does no type checking. It is designed to be interrupt safe and can be used
     asynchroneously. For example and interrupt routine can feed it while a
     process empties it without any problems.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __BOFIFO_H
#define __BOFIFO_H

#include "bomemory.h"

// This class implements a low level typeless FIFO mechanism for blocks
// of data. Each FIFO entry can consist of user defined number of subblocks
// that pass through the FIFO as a single entity. No memory moves are done
// in the FIFO so as to maximize performace.
// When an overrun or underrun occurs a sticky flag is set, the user can
// reset this flag at any time. The FIFO will continue working after an
// underrun	or an overrun.

class BoBlockfifo
	{
	public:
		enum State {LOCK, FULL, EMPTY};	// Possible states for a FIFO entry
	private:
		struct Level             // FIFO block descriptor
			{
			long loff;			 // Offset into fifo_buf of the current block
			State state;		 // State flag of the current block
			};
		BoMemory<char> fifo_buf; // Memory block holding the FIFO information
		BoMemory<Level> levp; 	 // Array of FIFO block descriptors
		long levels;			 // Number of entries the FIFO can hold
		short secs;				 // Number of subblock in a FIFO entry
		long inp;				 // Offset for adding data into the FIFO
		long outp;				 // Offset for taking data out of the FIFO
	public:
		short overrun;			 // Overrun sticky flag	0=OK 1=overrun
		short underrun;			 // Underrun sticky flag 0=OK 1=underrun
		void *inptr;        // Pointer to FIFO data set by access_in
		void *outptr;       // Pointer to FIFO data set by access_out
		BoMemory<long> offs, s;	 // Array of offsets and sizes for subblocks

		BoBlockfifo (long bytes = 0L, short flags = BoAlloc::NORMAL);
		void resize (long bytes);
		long init (short align, const BoMemory<long> &sizes);
		long get_levels ();
		void push (const BoAlloc &p, ...);
		void push (const BoMemory<BoAlloc *> &p);
		void pull (BoAlloc &p, ...);
		void pull (BoMemory<BoAlloc *> &p);
		void allocate ();
		void release ();
		void unlock (short num = 0);
		State access_in (short num = 0);
		State access_out (short num = 0);
	};

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::BoBlockfifo
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        BoBlockfifo::BoBlockfifo (long bytes, short flags);

        bytes           Number of bytes of memory to reserve for the FIFO
		flags           Memory property flags for BoMemory

Description
        This constructor build a FIFO object with a specified number of
		bytes. The number of bytes can be changed with the resize function.
		Before the FIFO can be used the init function must be called.
 #$%!........................................................................*/

inline BoBlockfifo::BoBlockfifo (long bytes, short flags) :
	fifo_buf (bytes, flags), levp (0, flags), offs (0, flags), s (0, flags)
{
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::resize
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBlockfifo::resize (long bytes);

        bytes           Number of bytes of memory to reserve for the FIFO

Description
        This function changes the number of bytes reserved by a FIFO.
		After resize is used the init function must be called to reinitialize
		the FIFO. All data in the FIFO is lost.
 #$%!........................................................................*/

inline void BoBlockfifo::resize (long bytes)
{
	fifo_buf.resize (bytes);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::get_levels
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        long BoBlockfifo::get_levels ();

		returns         The number of FIFO entries currently available.

Description
        This function is used to obtain the number of FIFO entries are
		available. The result of this function is undefined until init
		is called.
 #$%!........................................................................*/

inline long BoBlockfifo::get_levels ()
{
	return (levels);
}
#endif

