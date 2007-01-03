/* BO_INTR.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BO_INTR.H 19-Jun-94,20:26:28,`THOMAS' File creation
1:1 BO_INTR.H 30-Jun-94,19:51:12,`THOMAS'
     Changed the definition of the interrupt routine from
     void interrupt func (...) to void interrupt func ()
     and fixed things so that both Borland C++ and Watcom C++ can
     compile this code
1:2 BO_INTR.H 11-Jul-94,17:15:06,`THOMAS' Modified to work in a DLL
1:3 BO_INTR.H 26-May-95,18:16:02,`TBUIJS'
     The name of the object is now BoInterrupt.
1:4 BO_INTR.H 6-Jul-95,20:19:34,`TBUIJS'
     Added function headers to the inline functions.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __BO_INTR_H
#define __BO_INTR_H

#include <conio.h>
#ifdef __QNX__
#include <sys/types.h>
#endif

class BoInterrupt
	{
#ifndef __QNX__
		static unsigned short irq_to_int[16];
#endif
		short irq_num;
                int my_iid;
#ifndef __QNX__
		void interrupt (far *oldr) ();
		void interrupt (far *oldp) ();
#endif
	public:
#ifdef __QNX__
		BoInterrupt (short int_num);
#else
		BoInterrupt (short int_num, void interrupt (*handler) ());
#endif
		~BoInterrupt ();
		void mask ();
#ifndef __QNX__
		void unmask ();
#else
		void unmask (pid_t far ( * handler) (void));
#endif
		void eoi ();
	};

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoInterrupt::eoi
File:   BO_INTR.H
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        void BoInterrupt::eoi ();

Description
		This function must be called by the interrupt service routine to
		signal the end of the interrupt to the interrupt controller.
 #$%!........................................................................*/

inline void BoInterrupt::eoi ()
{
#ifndef __QNX__
	// send End Of Interrupt signal to 8259
	if (irq_num < 8)
		{
		(void) outp (0x20, 0x20); // send EOI
		}
	else
		{
		(void) outp (0xa0, 0x20); // send EOI to master
		(void) outp (0x20, 0x20); // send EOI to slave
		}
#endif
}

#endif



