/* BOFIFO.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BOFIFO.CPP 16-Jun-95,17:36:42,`TBUIJS'
     New Large untyped FIFO management object used to implement background multi
     acquisitions.
1:1 BOFIFO.CPP 11-Jul-95,11:50:28,`TBUIJS'
     Added a check to see in the FIFO is big enough to hold 1 level,
     if it is not an exception is generated.
1:2 BOFIFO.CPP 8-Nov-95,19:42:26,`TBUIJS'
     The code for wrapping around the end of the array that is used for the FIFO
     was incorrect, it read

     advance + current_position % array_size

     when this should have been

     (advance + current_position) % array_size
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stdarg.h>

#include "bomemory.h"
#include "bofifo.h"

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::init
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        long BoBlockfifo::init (short align, const BoMemory<long> &sizes);

        align           All subblocks and block will be on a multiple of
		                align. This must be done to avoid having an element
						stradle a segment boundary in 16 bit mode.
		sizes           array containing the size of each subblock. The
		                number of subblocks is determined by the size of
						the sizes object.

		returns         The number of entries in the FIFO

Description
		This function is used to initialize the FIFO given the size of each
		subblock in a FIFO level. The number of FIFO entries is returned.
		The init function must be called before using the FIFO for the first
		time. It must also be called after using the resize function.
 #$%!........................................................................*/

long BoBlockfifo::init (short align, const BoMemory<long> &sizes)
{
	long i;

	secs = (short)sizes.size ();
	s = sizes;
	offs.resize (secs);

	// compute the offsets for the subblocks bassed on the sizes and on the
	// alignment requirement
	offs.p[0] = 0;
	for (i = 1; i < secs; i++)
		{
		offs.p[i] = offs.p[i-1] + sizes.p[i-1];
		if (offs.p[i] % align != 0)
			{
			offs.p[i] += align - offs.p[i] % align;
			}
		}

	// compute the size of the FIFO entries or levels, make sure that
	// alignment criteria are maintained between levels. Also determine the
	// number of levels available for the current level size
	long level_size = offs.p[secs-1] + sizes.p[secs-1];
	if (level_size % align != 0)
		{
		level_size += align - level_size % align;
		}
	if ((levels = fifo_buf.size () / level_size) == 0)
		{
		throw (BoError (FIFO_TOO_SMALL));
		}
	inp = outp = 0;
	overrun = 0;
	underrun = 0;

	// initialize the FIFO block descriptors to EMPTY and compute the offsets
	// to access each level from the beggining of fifo_buf.
	levp.resize (levels);
	for (i = 0; i < levels; i++)
		{
		levp.p[i].loff  = i*level_size;
		levp.p[i].state = EMPTY;
		}
	return (levels);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::push
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBlockfifo::push (const BoAlloc &p, ...)

        p               Data for first subblock of the top entry
		...             A series of BoAlloc * objects, one for each 
		                subblock to be initialized. No checking is done
						so there must be exactly the correct number of
						objects to match the number of subblocks

Description
        This function is used to add a new entry into the FIFO. The data
		for each of the subblocks making up the entry is copied from the
		BoAlloc pointers passed in the argument list. The block is marked as
		FULL and can be removed from the FIFO by the pull function. If the
		FIFO is full an overrun is signalled and the operation is aborted.
 #$%!........................................................................*/

void BoBlockfifo::push (const BoAlloc &p, ...)
{
	va_list lst;
	
	// check for overrun condition
	if (levp.p[inp].state != EMPTY)
		{
		overrun = 1;
		return;
		}

	va_start (lst, p);

	// copy data from BoMemory objects to FIFO blocks
	fifo_buf.copy_raw_block (p, levp.p[inp].loff + offs.p[0], 0, s.p[0]);
	for (short i = 1; i < secs; i++)
		{
		fifo_buf.copy_raw_block (*va_arg (lst, BoAlloc *),
							 levp.p[inp].loff + offs.p[i], 0, s.p[i]);
		}

	va_end (lst);

	// mark slot as FULL and advance in pointer to next free slot
	levp.p[inp++].state = FULL;
	if (inp == levels)
		{
		inp = 0;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::push
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBlockfifo::push (const BoMemory<BoAlloc *> &p)

        p               Array of BoAlloc objects

Description
        This function is used to add a new entry into the FIFO. The data
		for each of the subblocks making up the entry is copied from the
		BoAlloc array passed as an argument. The block is marked as
		FULL and can be removed from the FIFO by the pull function. If the
		FIFO is full an overrun is signaled and the operation is aborted.
 #$%!........................................................................*/

void BoBlockfifo::push (const BoMemory<BoAlloc *> &p)
{
	// check for overrun condition
	if (levp.p[inp].state != EMPTY)
		{
		overrun = 1;
		return;
		}

	// copy data from BoMemory objects to FIFO blocks
	for (short i = 0; i < secs; i++)
		{
		fifo_buf.copy_raw_block (*p.p[i], levp.p[inp].loff + offs.p[i],
									0, s.p[i]);
		}

	// mark slot as FULL and advance in pointer to next free slot
	levp.p[inp++].state = FULL;
	if (inp == levels)
		{
		inp = 0;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::pull
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBlockfifo::pull (BoAlloc &p, ...)

        p               Memory objects to hold the data from the first
		                subblock ob the object comming out of the FIFO
		...             A series of BoAlloc * objects, one to hold the data
		                of each subblock. No checking is done
						so there must be exactly the correct number of
						objects to match the number of subblocks

Description
        This function is used to remove the oldest entry in the FIFO. The
		data in each of the subblocks making up the entry is copied to the
		BoAlloc pointers passed in the argument list. The block is marked as
		EMPTY and can be reused. The sizes of the BoAlloc objects are
		automatically adjusted, if required, to hold the needed amount
		of data. If no block is available on the FIFO an underrun is signaled
		and the operation is aborted. 
 #$%!........................................................................*/

void BoBlockfifo::pull (BoAlloc &p, ...)
{
	va_list lst;
	
	// check for underrun condition
	if (levp.p[outp].state == EMPTY || levp.p[outp].state == LOCK)
		{
		underrun = 1;
		return;
		}

	va_start (lst, p);

	// make sure the block size is right
	if (s.p[0] % p.esize () == 0)
		{
		p.resize (s.p[0]/p.esize ());
		}
	else
		{
		p.resize (s.p[0]/p.esize () + 1);
		}

	// copy data from FIFO to the first BoAlloc object
	p.copy_raw_block (fifo_buf, 0, levp.p[outp].loff+offs.p[0], s.p[0]);

	// adjust the size and copy data to the other BoAlloc objects
	BoAlloc *pp;
	for (short i = 1; i < secs; i++)
		{
		pp = va_arg (lst, BoAlloc *);
		if (s.p[i] % pp->esize () == 0)
			{
			pp->resize (s.p[i]/pp->esize ());
			}
		else
			{
			pp->resize (s.p[i]/pp->esize () + 1);
			}
		pp->copy_raw_block (fifo_buf, 0, levp.p[outp].loff+offs.p[i], s.p[i]);
		}

	va_end (lst);

	// mark slot as EMPTY and advance out pointer to next slot
	levp.p[outp++].state = EMPTY;
	if (outp == levels)
		{
		outp = 0;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::pull
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBlockfifo::pull (BoMemory<BoAlloc *> &p)

		p               Array of BoAlloc * objects, one to
		                hold the data of each subblock. No checking is done
						so there must be exactly the correct number of
						objects to match the number of subblocks

Description
        This function is used to remove the oldest entry in the FIFO. The
		data in each of the subblocks making up the entry is copied to the
		BoAlloc pointers passed as an array argument. The block is marked as
		EMPTY and can be reused. The sizes of the BoAlloc objects are
		automatically adjusted, if required, to hold the needed amount
		of data. If no block is available on the FIFO an underrun is signaled
		and the operation is aborted.
 #$%!........................................................................*/

void BoBlockfifo::pull (BoMemory<BoAlloc *> &p)
{
	// check for underrun condition
	if (levp.p[outp].state == EMPTY || levp.p[outp].state == LOCK)
		{
		underrun = 1;
		return;
		}

	// adjust the size and copy data to the BoAlloc objects
	for (short i = 0; i < secs; i++)
		{
		if (s.p[i] % p.p[i]->esize () == 0)
			{
			p.p[i]->resize (s.p[i]/p.p[i]->esize ());
			}
		else
			{
			p.p[i]->resize (s.p[i]/p.p[i]->esize () + 1);
			}
		p.p[i]->copy_raw_block (fifo_buf, 0, levp.p[outp].loff + offs.p[i],
								s.p[i]);
		}

	// mark slot as EMPTY and advance out pointer to next slot
	levp.p[outp++].state = EMPTY;
	if (outp == levels)
		{
		outp = 0;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::allocate
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBlockfifo::allocate ()

Description
        This function acts like push to reserve the top entry in the FIFO
		for data but does not copy any data. The inptr pointer is adjusted
		to point to the beggining of the entry and the subblocks can be
		accessed by useing the offs and s arrays. The FIFO entry is marked
		as locked because the data has not yet been transfered. When the data
		transfer is completed by the caller it must use unlock to alloc the
		block to leave the FIFO.
Cautions
        The inptr pointer is only valid until the next call to allocate,
		unlock and access_in. These functions change the value of the inptr
 #$%!........................................................................*/

void BoBlockfifo::allocate ()
{
	// check for overrun condition
	if (levp.p[inp].state != EMPTY)
		{
		access_in (); // point pointers to current top on overrun
		overrun = 1;
		return;
		}

	// set pointer to FIFO block
	inptr = (void *)(fifo_buf.p + levp.p[inp].loff);

	// mark slot as LOCK and advance in pointer to next free slot
	levp.p[inp++].state = LOCK;
	if (inp == levels)
		{
		inp = 0;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::release
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBlockfifo::release ()

Description
        This function is used to release the oldest FIFO block much as pull
		does but with no data transfer. Presumably the data was recuperated
		with access_out before release was called. Release will signal
		in underrun if there is no block waiting in the FIFO or if the oldest
		block is maked LOCK.
 #$%!........................................................................*/

void BoBlockfifo::release ()
{
	// check for underrun condition
	if (levp.p[outp].state == EMPTY || levp.p[outp].state == LOCK)
		{
		underrun = 1;
		return;
		}

	// mark slot as EMPTY and advance out pointer to next slot
	levp.p[outp++].state = EMPTY;
	if (outp == levels)
		{
		outp = 0;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::unlock
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoBlockfifo::unlock (short num = 0)

		num                   number of the entry to unlock counting from
		                      the most recent to the oldest entry. By
							  default if num is not specified the most
							  recent entry is unlocked.

Description
        This function is used to unlock a block that was allocated with 
		allocate so that it can be removed from the FIFO by the pull or
		release functions. It sets inptr to point to the block just unlocked.
Cautions
        The inptr pointer is only valid until the next call to allocate,
		unlock and access_in. These functions change the value of the inptr

 #$%!........................................................................*/

void BoBlockfifo::unlock (short num)
{
	long level = inp-num-1;

	if (level < 0)
		{
		// math definition for level%levels when level is negative
		level = levels-(-level)%levels;
		}
	// check if entry valid
	if (levp.p[level].state	== EMPTY)
		{
		// error
		return;
		}

	// unlock FIFO block
	levp.p[level].state = FULL;

	// set pointer to FIFO block
	inptr = (void *)(fifo_buf.p + levp.p[level].loff);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::access_in
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        BoBlockfifo::State BoBlockfifo::access_in (short num = 0)

		num                   number of the entry to access counting from
		                      the most recent to the oldest entry. By
							  default if num is not specified the most
							  recent entry is accessed.

		returns               The state of the accessed entry. If the state
		                      is EMPTY the data in the block should not be
							  used or updated. Any data put into an empty
							  block will probably be overwritten and lost.
Description
        This function is used to gain access to the data in a block in the
		FIFO. The block is specified by num, the number of entries starting
		from the newest. By default the newest entry is accessed.
		inptr is set to point to the access block.
Cautions
        The inptr pointer is only valid until the next call to allocate,
		unlock and access_in. These functions change the value of the inptr

 #$%!........................................................................*/

BoBlockfifo::State BoBlockfifo::access_in (short num)
{
	long level = inp-num-1;

	if (level < 0)
		{
		// math definition for level%levels when level is negative
		level = levels-(-level)%levels;
		}

	// set pointer to FIFO block
	inptr = (void *)(fifo_buf.p + levp.p[level].loff);

	return (levp.p[level].state);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoBlockfifo::access_out
File:   BOFIFO.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        BoBlockfifo::State BoBlockfifo::access_out (short num = 0)

		num                   number of the entry to access counting from
		                      the oldest to the most recent entry. By
							  default if num is not specified the oldest 
							  entry is accessed.

		returns               The state of the accessed entry. If the state
		                      is EMPTY the data in the block should not be
							  used or updated. Any data put into an empty
							  block will probably be overwritten and lost.
Description
        This function is used to gain access to the data in a block in the
		FIFO. The block is specified by num, the number of entries starting
		from the oldest. By default the oldest entry is accessed.
		outptr is set to point to the access block.
Cautions
        The outptr pointer is only valid until the next call to access_out.

 #$%!........................................................................*/

BoBlockfifo::State BoBlockfifo::access_out (short num)
{
	long level = (outp+num) % levels;

	// set pointer to FIFO block
	outptr = (void *)(fifo_buf.p + levp.p[level].loff);

	return (levp.p[level].state);
}


// ******************************************************************
// Test code
// ******************************************************************

#if 0
#include <stdlib.h>
#include <iostream.h>

#include "bomemory.h"
#include "bofifo.h"

main (int argc, char *argv[])
{
	if (argc != 3)
		{
		cout << "Supply the FIFO size and number of subsections\n";
		return (-1);
		}

	long i, j, k;
	BoBlockfifo fifo (atol (argv[1]));
	short blocks = atoi (argv[2]);
	BoMemory<long> sizes (blocks);
	randomize ();
	for (i = 0; i < blocks; i++)
		{
		sizes.p[i] = random (200) + 1;
		}

	long levels = fifo.init (4, sizes);

	BoMemory<BoMemory<unsigned char> > data (blocks);
	BoMemory<BoAlloc *> pdata (blocks);

	for (j = 0; j < blocks; j++)
		{
		data.p[j].resize (sizes.p[j]);
		pdata.p[j] = (BoAlloc *)&data.p[j];
		}

	for (i = 0; i < levels; i++)
		{
		for (j = 0; j < blocks; j++)
			{
			for (k = 0; k < sizes.p[j]; k++)
				{
				data.p[j].p[k] = (unsigned char)((k + i) % 255);
				}
			}
		fifo.push (pdata);
		}

	for (i = 0; i < levels; i++)
		{
		fifo.pull (pdata);
		cout << "\nLevel " << i << ": ";
		for (j = 0; j < blocks; j++)
			{
			cout << "sec " << j << "- ";
			for (k = 0; k < sizes.p[j]; k++)
				{
				cout << (short)data.p[j].p[k] << ", ";
				}
			}
		}

	cout << "\n overrun = " << fifo.overrun << ", underrun = " <<
							fifo.underrun << "\n";
	return (0);
}
#endif


