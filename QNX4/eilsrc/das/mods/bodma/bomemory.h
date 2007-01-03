/* BOMEMORY.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BOMEMORY.H 23-Jun-94,11:27:38,`THOMAS' File creation
1:1 BOMEMORY.H 27-Jun-94,15:33:16,`JEROMEC'
     Member functions are no longer 'inline' to avoid a bug in BC4
1:2 BOMEMORY.H 5-Jul-94,10:50:14,`THOMAS'
     Eliminated the use of the min macro in favor of an if statement because of
     problems with the transportability and availability of the min/max functions
     when in C++ mode. Not all versions of stdlib.h define the template version
     of min.max when in C++ mode.
1:3 BOMEMORY.H 5-Jul-94,16:28:22,`THOMAS'
     Reinstate the use of the min macro using the new minmax include file,
     the problem was due to windows.h defining minmax as a macro!!
1:4 BOMEMORY.H 18-Jul-94,12:18:18,`THOMAS'
     Fix bomemory to correctly export symbols when in a DLL, also change the
     default assumption from lock=0 to lock=1 for debugging purposes and to
     work around an apparent bug in the BC4 memory suballocator
1:5 BOMEMORY.H 28-Jul-94,12:15:50,`THOMAS'
     Changed the implementation of the Memory object, uses a second allocator
     class that works with a void * and provides memory allocation services to
     the template based memory object. Also reinstated the use of the borland
     memory allocation routines, if DLL are involved the DLL based runtime library
     must be used, otherwise allocating a block in a DLL and freeing it in the
     main or in another DLL will currupt the heap!!!
1:6 BOMEMORY.H 11-Aug-94,15:04:32,`THOMAS'
     Define _import as an empty string if not using Borlandc since this keyword
     is only support in BC4.
1:7 BOMEMORY.H 14-Jun-95,13:21:14,`TBUIJS'
     BoMemory is now a two level system with BoAlloc as the base. BoAlloc uses
     virtual functions so that some generic manipulations can be done with
     BoAlloc objects that are typeless. Also the BoMemory object wich is
     build around a template now correctly calls the constructors and destructors
     for complex objects that it encapsulates.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __BOMEMORY_H
#define __BOMEMORY_H

#include <stdlib.h>
#include <new.h>

#include "bo_error.h"

class BoAlloc
	{
		unsigned short offset; // offset of pointer in real mode (see alloc)
		long _size;			   // size of each data element
		void alloc (long npts, long size, short flags);
		void bo_free ();
	protected:
		char *ptr; 	// pointer to the data
		long _npts;			// number of elements
		short _flags;		// memory block properties (only LOCK defined)
		BoAlloc (long npts, long bsize, short flags);
		virtual void copy_ptr ();
	public:
		enum				// possible values for flags
			{
			NORMAL = 0,
			LOCK = 1
			};
		BoAlloc (const BoAlloc & block);
		BoAlloc &operator = (const BoAlloc & block);
		void *pointer () const;
		long size () const;
		long esize () const;
		virtual void resize (long npts);
		void init (const char *val);
		void init_block (const char *val, long start, long npts);
		void move (BoAlloc &block);
		void copy_raw_block (const BoAlloc & block, long d, long s, long by);
		void copy_block (const BoAlloc &block, long d, long s, long npts);
		virtual ~BoAlloc ();
	};

template <class T> class BoMemory : public BoAlloc
	{
		void copy_ptr ();
	public:
		T *p;	
		BoMemory (long npts = 0L, short flags = NORMAL);
		BoMemory (const BoMemory & block);
		BoMemory &operator = (const BoMemory & block);
		void move (BoMemory &block);
		void init (const T &val);
		void init_block (const T &val, long start, long npts);
		void copy_block (const BoMemory &blk, long din, long sin, long npts);
		void resize (long npts);
		~BoMemory ();
	};

/**************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   new
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void *new (size_t, BoAlloc *mem);

		mem               Memory block to reuse

		returns           a void pointer to the same location mem points

Description
		This private version of new is used to reallocate an object on an
		existing piece of memory. This is used to call the constructor
		for the object without reallocating RAM for it.
 ........................................................................*/

inline void *operator new (size_t, BoAlloc *mem)
{
	return ((void *)mem);
}

/**************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::copy_ptr
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoAlloc::copy_ptr ();

Description
		This private virtual function is used to make sure that the pointer
		in BoMemory	is in synch with the pointer in BoAlloc. The BoAlloc
		version of copy_ptr does nothing since in a pure BoAlloc object
		the BoMemory pointer does not exist.
 ........................................................................*/

inline void BoAlloc::copy_ptr ()
{
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::BoAlloc
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        BoAlloc::BoAlloc (long npts, long size, long flags, void **p);

        npts            number of blocks to allocate
		bsize           size of each block
		flags           flags for memory block caracteristics
		p				pointer to the typed pointer in BoMemory

							possible values:
								BoAlloc::LOCK : memory locked in RAM

Description
        This constructor allocates a memory block that is npts * size bytes
 		long. It is typeless ancestor to Bomemory and is not directly
		accesible.
 #$%!i........................................................................*/

inline BoAlloc::BoAlloc (long npts, long bsize, short flags)
{
	alloc (npts, bsize, flags);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::move (BoAlloc & block)
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoAlloc::move (BoAlloc & block);

	    block           object encapsulating the block of memory to be moved

Description
        Moves the block of memory in block to the current object and
		resets block to empty.

 #$%!i........................................................................*/

inline void BoAlloc::move (BoAlloc &block)
{
	// are the two blocks compatible
	if (_size != block._size)
		{
		throw (BoError (INVALID_ARGUMENT));
		}

	bo_free ();
	_npts  = block._npts;
	ptr    = block.ptr;
	copy_ptr ();           // resynch pointers with virtual function
	_flags = block._flags;

	block.ptr   = NULL;
	block.copy_ptr ();     // resynch pointers with virtual function
	block._npts = 0;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::size
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        long BoAlloc::size () const;

	    returns the number of elements in the current object

Description
        This functions returns the number of elements in the current object.
 #$%!........................................................................*/

inline long BoAlloc::size () const
{
	return (_npts);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::esize
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        long BoAlloc::esize () const;

	    returns the size of the elements in the current object

Description
        This functions returns the size number of the elements in the
		current object.
 #$%!........................................................................*/

inline long BoAlloc::esize () const
{
	return (_size);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::pointer
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void huge *BoAlloc::pointer () const;

	    returns a pointer to the memory block

Description
        This functions returns a pointer to the memory block inside the
		current object.
 #$%!........................................................................*/

inline void *BoAlloc::pointer () const
{
	return ((void *)ptr);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoAlloc::~BoAlloc
File:   BOMEMORY.CPP
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        BoAlloc::~BoAlloc ();

Description
        The destructor for the BoAlloc class. it releases any dynamic
		memory associated with the current instance of BoAlloc and
		indirectly BoMemory. The function cannot be called directly, it
		is called automatically when the object goes out of scope.
 #$%!i........................................................................*/

inline BoAlloc::~BoAlloc ()
{
	bo_free ();
}

/**************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::copy_ptr
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoMemory<T>::copy_ptr ();

Description
		This private virtual function is used to make sure that the pointer
		in BoMemory	is in synch with the pointer in BoAlloc.
 ........................................................................*/

template <class T> void BoMemory<T>::copy_ptr ()
{
	p = (T *)ptr;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::BoMemory
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        BoMemory<T>::BoMemory (long npts, short flags);

        npts            number of elements to allocate
		flags           flags for memory block caracteristics
							possible values:
								BoAlloc::NORMAL : normal memory block
								BoAlloc::LOCK : memory locked in RAM

Description
        This constructor allocates a memory buffer big enough to contain
        npts elements of size sizeof(T). It create an object that
        encapsulates a pointer to type T using a template. The lock flag
        is used to indicate whether the memory should be locked in RAM
        or not, this flag is useful for environments with virtual memory.
 #$%!........................................................................*/

template <class T> BoMemory<T>::BoMemory (long npts, short flags) :
	BoAlloc (npts, sizeof (T), flags), p ((T *)pointer ())
{
	// need to initialize each element in the array because it might have
	// a constructor that needs to be called
	T *pp = p;
	for (long i = npts+1; --i; pp++)
		{
		new ((BoAlloc *)pp) T;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::BoMemory
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        BoMemory<T>::BoMemory (const BoMemory<T> & block);

		block           block to copy into the current block.

Description
        This function will make a copy of memory buffer block into the current
        memory buffer. The contents of the current block is destroyed and
        the current block's size is adjusted automatically to correspond to
        the	size of the block being copied. This function will be called
		automatically whenever a copy is needed, it is a copy constructor.
 #$%!........................................................................*/

template <class T> BoMemory<T>::BoMemory (const BoMemory<T> & block) :
	BoAlloc (block), p ((T * const)ptr)
{
	// need to call the copy constructors to finish the copy process on the
	// subblocks. This results in the copy being done twice.
	T *pp1 = p, *pp2 = block.p;
	for (long i = _npts+1; --i; pp1++, pp2++)
		{
		*pp1 = *pp2;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::operator =
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        BoMemory &BoMemory<T>::operator = (const BoMemory<T> & block);

		block           block to copy into the current block.

Description
        This operator will make a copy of memory buffer block into the
		current memory buffer. The contents of the current block is destroyed
		and the current block's size is adjusted automatically to correspond
		to the size of the block being copied. A reference to the copied
		block is returned so that assignments can be cascaded.
 #$%!........................................................................*/

template <class T> BoMemory<T> &BoMemory<T>::operator = (
	const BoMemory<T> & block)
{
	BoAlloc *tmp = this;
	*tmp = block; // required to reallocate memory

	T *pp1 = p, *pp2 = block.p;
	for (long i = _npts+1; --i; pp1++, pp2++)
		{
		*pp1 = *pp2;
		}

	return (*this);
}

/*#$%!*************************************************************************
							COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::move
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   January, 1995

Synopsis
		  void BoMemory<T>::move (BoMemory<T> & block);

		block           block to move into the current block

Description
		  This function moves the memory buffer contained in block to the
		  current BoMemory object. block is reset to empty and the prior
		  contents of the current buffer is lost.
 #$%!........................................................................*/

template <class T> void BoMemory<T>::move (BoMemory<T> & block)
{
	BoAlloc::move (block);
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::init
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoMemory<T>::init (const T & val);

		val             value to initialize the block with

Description
        This function will set all the elements of the memory block to val,
        where val is any value of type T; the type of the memory block.
 #$%!........................................................................*/

template <class T> void BoMemory<T>::init (const T & val)
{
	T *pp = p; 
	for (long i = _npts+1; --i; pp++)
		{
		*pp = val;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::init_block
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoMemory<T>::init_block (const T & val, long start, long npts);

		val             value to initialize the region with
        start           starting index of the region to initialize
        npts            length of the region to initialize

Description
        This function initializes a region of a memory block to value val,
        the region starts at index start and goes on for npts elements. If
        the region defined be start and npts is not contained in the memory
        block an INVALID_INDEX eception is thrown.
 #$%!........................................................................*/

template <class T> void BoMemory<T>::init_block (const T & val,
													long start, long npts)
{
	if (start+npts > _npts)
		{
		throw (BoError (INVALID_INDEX));
		}
	T *pp = p+start; 
	for (long i = npts+1; --i; pp++)
		{
		*pp = val;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::copy_block
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   January, 1995

Synopsis
        void BoMemory<T>::copy_block (const BoMemory<T> & block, long din,
                                      long sin, long npts);

		block           memory buffer to copy from
        din             index to start copying to in the destination
        sin             index to start copying from in the source
        npts            number of elements to copy

Description
        This function copies npts elelemts from memory buffer block starting
        at index sin in block to the current memory buffer starting at index
        din. If the copy operation does not fit in the source block or the
        destination block and INVALID_INDEX exception is thrown.
 #$%!........................................................................*/

template <class T> void BoMemory<T>::copy_block (
					const BoMemory<T> & block, long din, long sin, long npts)
{
	// compute size of block to copy; that is the minimum of
	// npts, block._npts-sin and _npts-din
	long bsize = npts;
	if (bsize > block._npts-sin)
		{
		bsize = block._npts-sin;
		}
	if (bsize > _npts-din)
		{
		bsize = _npts-din;
		}

	if (bsize < 0)
		{
		throw (BoError (INVALID_INDEX));
		}

	if (bsize == 0) return;

	T *pp1 = p+din, *pp2 = block.p+sin;
	for (long i = bsize+1; --i; pp1++, pp2++)
		{
		*pp1 = *pp2;
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::resize
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoMemory<T>::resize (long npts);

		npts            new number of points for object

Description
		This function changes the size of an object so that it can contain
		npts points. The original elements at the beggining of the array are
		unchanged. If any elements are added the constructor for those
		elements is called. If any elements are removed the destructor for
		them is also called.
 #$%!........................................................................*/

template <class T> void BoMemory<T>::resize (long npts)
{
	// if npts is the same as the current size no action is taken
	// case where the object gets bigger
	if (npts > _npts)
		{
		long tnpts = _npts; // copy _npts because resize() changes it
		BoAlloc::resize (npts);
		T *pp = p+tnpts;
		for (long i = npts-tnpts+1; --i; pp++)
			{
			pp = new ((BoAlloc *)pp) T;	 // function returns it's argument
			}
		}
	// case where the object gets smaller
	else if (npts < _npts)
		{
		T *pp = p+npts;
/*		for (long i = _npts-npts+1; --i; pp++)
			{
			pp->T::~T();
			}
*/
		BoAlloc::resize (npts);
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMemory<T>::~BoMemory
File:   BOMEMORY.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
        void BoMemory<T>::~BoMemory ()

Description
		This is the destructor for BoMemory. It makes sure that the
		destructor of each element in the memory block is called.
 #$%!........................................................................*/

template <class T> BoMemory<T>::~BoMemory ()
{
	T *pp = p;
/*	for (long i = _npts+1; --i; pp++)
		{
		pp->T::~T();
		}
*/
}

#endif
