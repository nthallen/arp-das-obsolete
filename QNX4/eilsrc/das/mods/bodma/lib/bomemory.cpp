/* BOMEMORY.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 MEMORY.CPP 20-Jun-94,18:38:04,`THOMAS' File creation
1:1 BOMEMORY.CPP 5-Jul-94,10:58:34,`THOMAS'
     Eliminated the use of the min macro in favor of an if statement because of
     problems with the transportability and availability of the min/max functions
     when in C++ mode. Not all versions of stdlib.h define the template version
     of min.max when in C++ mode.
     Changed the name of memory.cpp to bomemory.cpp in order to make the names
     of headers and .cpp modules more easily associable.
     Add support for WATCOM c style memory allocation with the halloc functions.
1:2 BOMEMORY.CPP 5-Jul-94,16:28:22,`THOMAS'
     Reinstate the use of the min macro using the new minmax include file,
     the problem was due to windows.h defining minmax as a macro!!
1:3 BOMEMORY.CPP 17-Jul-94,18:29:00,`THOMAS'
     Fix bomemory to correctly export symbols when in a DLL, also change the
     default assumption from lock=0 to lock=1 for debugging purposes and to
     work around an apparent bug in the BC4 memory suballocator
1:4 BOMEMORY.CPP 18-Jul-94,19:38:00,`THOMAS'
     This version corrects a problem that prevented bomemory from compiling under
     DOS!
1:5 BOMEMORY.CPP 28-Jul-94,12:13:36,`THOMAS'
     Changed the implementation of the Memory object, uses a second allocator
     class that works with a void * and provides memory allocation services to
     the template based memory object. Also reinstated the use of the borland
     memory allocation routines, if DLL are involved the DLL based runtime library
     must be used, otherwise allocating a block in a DLL and freeing it in the
     main or in another DLL will currupt the heap!!!
1:6 BOMEMORY.CPP 2-Aug-94,16:54:22,`THOMAS'
     Fixed resize so that resize to npts = 0 simply does a free and resize with
     no change in size does nothing.
1:7 BOMEMORY.CPP 10-Aug-94,8:58:18,`JEROMEC'
     Corrected a small bug in Bomemory::init_block that prevented intitializing
     the whole memory block (changed a >= for a >)
1:8 BOMEMORY.CPP 16-Jun-95,19:40:52,`TBUIJS'
     The code was changed extensively. It is now possible to use BoAlloc as a
     base class for BoMemory and not run into troubles. The pointer in BoMemory
     is updated with a virtual function so that everything works even when
     BoAlloc objects are used. The destructor is also virtual. New code was
     added so that if a Bomemory block containing complex objects is created the
     contructors and destructors for each element are called correctly. This
     is also true for resize and copy functions.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stdlib.h>
#include <string.h>
#include <mem.h>
#include <malloc.h>
#include <dos.h>

#include "bomemory.h"
#include "bo_error.h"

void bigmemmove (char *dest, char *src, long length);
void bigmemset (char *dest, int val, long length);

/**************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::alloc
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoAlloc::alloc (long npts, long bsize, char flags);

        npts            number of blocks to allocate
		bsize           size in bytes of each block
		flags           flags for memory block caracteristics
							possible values:
								BoAlloc::LOCK : memory locked in RAM

Description
        This function is used to perform the actual memory allocation
		of blocks for BoAlloc and BoMemory clases. This function is
		private to BoAlloc and cannot be used directly.
 ........................................................................*/
#if defined(__QNX__)
void BoAlloc::alloc (long npts, long bsize, short flags)
{
	ptr = NULL;

	if (npts != 0)
		{
		if ((ptr = (char *)malloc (npts * bsize)) == NULL)
			{
			throw (BoError (NOT_ENOUGH_MEMORY));
			}
		}

	copy_ptr ();           // resynch pointers with virtual function
	_npts = npts;
	_size = bsize;
	_flags = flags;
	offset = 0;
}
#endif

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::bo_free
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoAlloc::bo_free ()

Description
        This function releases the current memory block. It is normally
		called by BoAlloc::~BoAlloc when an object is destroyed.
 #$%!i........................................................................*/
#if defined(__QNX__)
void BoAlloc::bo_free ()
{
	if (ptr == NULL || _npts == 0 || _size == 0) return;
	free (ptr);
	ptr = NULL;
	copy_ptr ();           // resynch pointers with virtual function
	_npts = 0;
}
#endif

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::BoAlloc
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        BoAlloc::BoAlloc (const BoAlloc & block)

        block           Memory block to copy

Description
        This function creates a new copy of the memory block encapsulated
		in block. Block is not affected in any way by this function.
 #$%!i........................................................................*/

BoAlloc::BoAlloc (const BoAlloc & block)
{
	alloc (block._npts, block._size, block._flags);
	bigmemmove (ptr, block.ptr, _npts * _size);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::operator =
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        BoAlloc &BoAlloc::operator = (const BoAlloc & block)

        block           Memory block to copy

Description
        This operator copies the contents of the memory block encapsulated
		in block into the current block. The size of the current block is
		adjusted to be the same size as the block being copyed. block is
		not affected in any way by this function. A reference to the block
		that was copied is returned so that assignments can be cascaded.
 #$%!i........................................................................*/

BoAlloc &BoAlloc::operator = (const BoAlloc & block)
{
	// are the two blocks compatible
	if (_size != block._size)
		{
		throw (BoError (INVALID_ARGUMENT));
		}
	resize (block._npts);
	bigmemmove (ptr, block.ptr, _npts * _size);
	return (*this);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::init
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoAlloc::init (const char *val);

        val             Value at which the buffer will be set

Description
        This function initialises the entire buffer to the value pointed
		to by val. val is assumed to point to a block having the same number
		of bytes as the block size of the memory buffer.
 #$%!i........................................................................*/

void BoAlloc::init (const char *val)
{
	long i;

	// if the data type is 1 byte long we can simply set the whole array
	if (_size == 1)
		{
		bigmemset (ptr, *val, _npts);
		return;
		}

	// if data type > 1 byte but the value has all bytes equal set the array
	for (i = 1; i < _size; i++)
		{
		if (*val != ((char *)val)[i]) goto general;
		}
	bigmemset (ptr, *val, _npts*_size);
	return;

	// general case is slower but always works; it copy a series of blocks
general:
	char *t = ptr;
	for (i = _npts+1; --i; t += _size)
		{
		memcpy (t, val, (size_t)_size);
		}
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::init_block
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoAlloc::init_block (const char *val, long start, long npts);

        val             Value at which the buffer will be set
		start           starting block to initialise
		npts            number of block to initialise

Description
        This function initalises the memory buffer to the value pointed to
		by val starting at block start for npts blocks. If npts blocks
		is longer than the size of the part of the buffer following start
		or if start is outside the buffer an INVALID_INDEX exception is
		thrown.
 #$%!i........................................................................*/

void BoAlloc::init_block (const char *val, long start, long npts)
{
	if (start+npts > _npts)
		{
		throw (BoError (INVALID_INDEX));
		}
	long i;

	// if the data type is 1 byte long we can simply set the whole array
	if (_size == 1)
		{
		bigmemset (ptr + start, *val, npts);
		return;
		}

	// if data type > 1 byte but the value has all bytes equal set the array
	for (i = 1; i < _size; i++)
		{
		if (*val != ((char *)val)[i]) goto general;
		}
	bigmemset (ptr + start*_size, *val, npts*_size);
	return;

	// general case is slower but always works; it copy a series of blocks
general:
	char *t = ptr + start*_size;
	for (i = npts+1; --i; t += _size)
		{
		memcpy (t, val, (size_t)_size);
		}
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::copy_raw_block
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoAlloc::copy_raw_block (const BoAlloc &block, long din,
		                              long sin, long bytes)

        block           Block to copy from
		din             position in current block to start copying at
		sin             position in block to start copying from
		bytes           number of bytes to copy

Description
        This function will copy a block from memory object block to
		the current memory object. The copy does not check for type
		compatibility like copy_block. The copy starts at sin bytes in the
		source block and at din bytes in the destination block regardles of
		the element sizes.

 #$%!i........................................................................*/

void BoAlloc::copy_raw_block (const BoAlloc & block, long d, long s, long by)
{
	// compute size of block to copy; that is the minimum of
	// bytes, block._npts*block._size-s and _npts*_size-d
	if (by > block._npts*block._size-s)
		{
		by = block._npts*block._size-s;
		}
	if (by > _npts*_size-d)
		{
		by = _npts*_size-d;
		}

	if (by <= 0) return;
	bigmemmove (ptr + d, block.ptr + s, by);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::copy_block
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoAlloc::copy_block (const BoAlloc &block, long din, long sin,
                                   long npts)

        block           Block to copy from
		din             position in current block to start copying at
		sin             position in block to start copying from
		npts            number of points to copy

Description
        This function will copy a block from memory object block to
		the current memory object. The copy starts at index sin in the
		source block and at index din in the destination block. npts points
		are copyed, each point is size bytes long where size is the size
		of the elements in the current memory buffer.

 #$%!i........................................................................*/

void BoAlloc::copy_block (const BoAlloc & block, long d, long s, long npts)
{
	// are the two blocks compatible
	if (_size != block._size)
		{
		throw (BoError (INVALID_ARGUMENT));
		}

	// compute size of block to copy; that is the minimum of
	// npts, block._npts-s and _npts-d
	long bsize = npts;
	if (bsize > block._npts-s)
		{
		bsize = block._npts-s;
		}
	if (bsize > _npts-d)
		{
		bsize = _npts-d;
		}

	if (bsize < 0)
		{
		throw (BoError (INVALID_INDEX));
		}

	if (bsize == 0) return;
	bigmemmove (ptr + d*_size, block.ptr + s*_size, bsize*_size);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::resize
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoAlloc::resize (long npts);

        npts            new number of points for the current block

Description
        This function changes the number of blocks to npts. The size of the
		blocks does not change.	This function can be used to increase or
		decrease the size of a memory allocation. The contents of the
		of the memory block does not change.
 #$%!........................................................................*/

#if defined(__QNX__)
void BoAlloc::resize (long npts)
{
	if (_npts == npts) return;
	if (npts == 0)
		{
		bo_free ();
		return;
		}
	if (ptr == NULL || _npts == 0)
		{
		alloc (npts, _size, _flags);
		return;
		}

	char *pp;

	if ((pp = (char *)malloc (npts * _size)) == NULL)
		{
		throw (BoError (NOT_ENOUGH_MEMORY));
		}
	bigmemmove (pp, ptr, (_npts > npts ? npts : _npts) * _size);
	free (ptr);

	_npts = npts;
	ptr   = pp;
	copy_ptr ();           // resynch pointers with virtual function
}
#endif

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BIGMEMSET
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   May 1, 1990

Synopsis
        void bigmemset (char huge *dest, int val, long length);

        dest            Buffer to be filled
        val             Value at which the buffer will be set
        length          Number of bytes to be filled

Description
        bigmemset is like memset in the C run time library, but it can
        handle arrays larger than 64K bytes since the size parameter is
        long.
 #$%!i........................................................................*/

void bigmemset (char *dest, int val, long length)
{
	if (length <= 0) return; /* nothing to do */
	memset (dest, val, (size_t)length);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BIGMEMMOVE
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   May 1, 1990

Synopsis
        void bigmemmove (char huge *dest, char huge *src, long length);

        dest            Destination buffer
        src             Source buffer
        length          Number of bytes to be copied

Description
        This function is like memmove in the standard C library but it allows
        for copying blocks of more than 64K bytes in 16bit implementations of
        C. This function can handle overlapping blocks correctly.
 #$%!i........................................................................*/

void bigmemmove (char *dest, char *src, long length)
{
	if (length <= 0) return; /* nothing to do */
	memmove(dest,src,(size_t)length);
}


// ******************************************************************
// Test code
// ******************************************************************

#if 0
#include <complex.h>
#include <iostream.h>

#include "bo_error.h"
#include "bomemory.h"

class A
	{
	public:
		int val;
		A ();
		~A ();
	};

A::A ()
{
	cout << "Constructed " << (void *)this << "\n";
}

A::~A ()
{
	cout << "Destructed " << (void *)this << "\n";
}

main (int argc, char *argv[])
{
	#if defined(_Windows) || defined(__WINDOWS__) /* win16 */
		#ifdef __BORLANDC__
			_InitEasyWin ();
		#endif
	#endif

	if (argc != 2)
		{
		cout << "Supply a block size for large allocation test\n";
		return (-1);
		}

	try
		{
		short i;

		BoMemory<short>	a(10), b(20), d;
		BoMemory<complex> c (20);
		BoMemory<A> test(5); // are constructors and destructors called
		BoMemory<char> big1 (atol (argv[1]), 1); // try a locked block
		big1.resize (10);
		BoMemory<char> big2 (atol (argv[1]));

		cout << "a:" << a.size () << " " << a.esize () << " " << a.pointer () << " " << a.p << "\n";
		cout << "b:" << b.size () << " " << b.esize () << " " << b.pointer () << " " << b.p << "\n";
		cout << "c:" << c.size () << " " << c.esize () << " " << c.pointer () << " " << c.p << "\n";
		cout << "d:" << d.size () << " " << d.esize () << " " << d.pointer () << " " << d.p << "\n";

		d = b;
		a.move (d);

		cout << "a:" << a.size () << " " << a.esize () << " " << a.pointer () << " " << a.p << "\n";
		cout << "b:" << b.size () << " " << b.esize () << " " << b.pointer () << " " << b.p << "\n";
		cout << "c:" << c.size () << " " << c.esize () << " " << c.pointer () << " " << c.p << "\n";
		cout << "d:" << d.size () << " " << d.esize () << " " << d.pointer () << " " << d.p << "\n";

		a.init (5);
		a.init_block (6, 5, 4);

		b.init (0);
		c.init (3.5);

		b.copy_block (a, 1, 3, a.size ()-5);

		for (i = 0; i < a.size (); i++)
			{
			cout << a.p[i] << " ";
			}
		cout << "\n";
	
		for (i = 0; i < b.size (); i++)
			{
			cout << b.p[i] << " ";
			}
		cout << "\n";
	
		c.resize (5);
		for (i = 0; i < c.size (); i++)
			{
			cout << c.p[i] << " ";
			}
		cout << "\n";

		// resize test for construction destruction
		test.resize (7);
		test.resize (3);

		cout << "End of main\n";
	
		return (0);
		}

	catch (BoError x)
		{
		cout << "Error " << x.number () << "\n";
		return (-1);
		}
}
#endif
