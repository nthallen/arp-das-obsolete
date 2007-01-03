/* BO_ERROR.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BO_ERROR.H 16-Jun-94,0:08:06,`THOMAS' File creation
1:1 BO_ERROR.H 4-Jul-94,12:02:44,`THOMAS'
     Changed the name of the error class from Error to Bo_error to avoid potential
     name clash problems and added a constructor and a print member to simplify
     exception handling with the Bo_error object
1:2 BO_ERROR.H 5-Jul-94,18:28:30,`THOMAS'
     Added new error codes INVALID_CONTEXT and TIMEOUT
1:3 BO_ERROR.H 6-Jul-94,8:24:46,`JEROMEC'
     Removed the just added TIMEOUT since it was already defined!
1:4 BO_ERROR.H 28-Jul-94,13:55:44,`THOMAS'
     Replaced the used of a dynamic string object with the use a fixed char array
     to avoid causing an infinite exception loop if the allocator fails when
     trying to allocate space for the error message.
1:5 BO_ERROR.H 19-Jun-95,17:53:42,`TBUIJS'
     The object is now called BoError, it has been simplified significantly and
     mow only containes an error code.
1:6 BO_ERROR.H 28-Jun-95,13:04:34,`TBUIJS' Added the DSP_ERROR code.
1:7 BO_ERROR.H 7-Jul-95,9:29:56,`TBUIJS' Added a new error code.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __BO_ERROR_H
#define __BO_ERROR_H

#include <iostream.h>
//#include <except.h>

// universal error class used for exception handling in Bomem C++ code
// should be ancestor to standard C++ error class when the standard is set
class BoError //: public xmsg
	{
		short num;	// error number
	public:
		BoError (short err_num = -1);
		short number () const;
	};

inline BoError::BoError (short err_num)
{
	num = err_num;
}

inline short BoError::number ()	const
{
	return (num);
}

// universal NO_ERROR and ERROR codes
const short NO_ERROR	  = 0;
const short GENERAL_ERROR = -1;

// low level unspecific error codes	(98 possible entries)
enum
	{
	INVALID_FILE = -99,			
	INVALID_DIRECTORY,		
	FILE_IO_ERROR,			
	FILE_NOT_FOUND,			
	NOT_ENOUGH_MEMORY,		
	NOT_ENOUGH_LOCKED_MEMORY,
	TIMEOUT,					
	TOO_MANY_FILES,			
	UNINITIALIZED,			
	INVALID_ID,				
	MISSING_PARAMETER,		
	INVALID_ARGUMENT, 		
	INVALID_INDEX,			
	INVALID_CONTEXT,			
	DMA_ERROR,
	NO_HARDWARE,
	DSP_ERROR,			
	ACQUISITION_ERROR,
	FIFO_UNDERRUN,
	FIFO_TOO_SMALL
	};
		
#endif
