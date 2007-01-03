/* BO_INTR.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BO_INTR.CPP 19-Jun-94,20:28:52,`THOMAS' File creation
1:1 BO_INTR.CPP 5-Jul-94,16:46:52,`THOMAS'
     Changed the definition of the interrupt routine from
     void interrupt func (...) to void interrupt func ()
     and fixed things so that both Borland C++ and Watcom C++ can
     compile this code
1:2 BO_INTR.CPP 6-Jul-94,15:31:14,`JEROMEC'
     Temporary addition of windows.h
1:3 BO_INTR.CPP 11-Jul-94,17:13:20,`THOMAS' Modified to work in a DLL
1:4 BO_INTR.CPP 15-Jun-95,8:04:28,`TBUIJS'
     The object name was changed to boInterrupt and the test code for the
     object is included in the code surrounded by #if 0 and #endif. A small
     changed was made in the way the interuppt is programmed to make the object
     compatible with OS/2 virtual DOS sessions. Also modification were made
     so that this object can be used correctly under DOS, 286 DOS extender,
     Borland PowerPack, Windows and DOS4GW. In all cases when a DOS extender is
     involved a passup interrupt is used. The code is compatible with both
     Borland C++ and Watcom C++.
1:5 BO_INTR.CPP 6-Jul-95,20:19:38,`TBUIJS'
     Added function headers for documentation.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#ifdef DOSX286
#include <phapi.h>
#endif

#include "bo_intr.h"

#ifndef __QNX__
unsigned short BoInterrupt::irq_to_int[16] = {0x8, 0x9, 0xa, 0xb, 0xc, 0xd,
				0xe, 0xf, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77};
#endif

// In Borland C++ interrupt functions are assumed to have type
//  void interrupt func (...)
// while in Microsoft C++ and Watcom C++ the type is
//  void interrupt func ()
// the following macro introduces a cast so that Borland C++ will accept
// interrupt functions defined as in Watcom C++

#ifdef __TURBOC__
#define _dos_getvect(a) ((void interrupt (*)())_dos_getvect (a))
#define _dos_setvect(a, b) (_dos_setvect (a, (void interrupt (*)(...))(b)))
#endif

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoInterrupt::BoInterrupt
File:   BO_INTR.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoInterrupt::BoInterrupt (short int_num, void interrupt (*handler) ())

		int_num         Hardware interrupt to hook
		handler         Interrupt service routine

Description
		This constructor associates an interrupt handler to a hardware
		interrupt.

 #$%!........................................................................*/

#ifdef __QNX__
BoInterrupt::BoInterrupt (short int_num)
#else
BoInterrupt::BoInterrupt (short int_num, void interrupt (*handler) ())
#endif
{
	BoInterrupt::irq_num = int_num;
	mask ();

	// program interrupt vector
#ifndef __QNX__
#ifndef DOSX286
	oldr = _dos_getvect (irq_to_int[int_num]);
	_dos_setvect (irq_to_int[int_num], handler);
#else
	DosSetPassToProtVec (irq_to_int[int_num], (PIHANDLER)handler,
									(PIHANDLER *)&oldp, (REALPTR *)&oldr);
#endif
#endif
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoInterrupt::~BoInterrupt
File:   BO_INTR.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        BoInterrupt::~BoInterrupt ()

Description
		The destructor masks the hardware interrupt and restores the original
		interrupt vector that was present when the object was created.

 #$%!........................................................................*/

BoInterrupt::~BoInterrupt ()
{
int e;
e = errno;
	// restore original handler
	mask ();
e = errno;
#ifdef __QNX__
	qnx_hint_detach(my_iid);
e = errno;
#else
#ifndef DOSX286
	_dos_setvect (irq_to_int[irq_num], oldr);
#else
	DosSetRealProtVec (irq_to_int[irq_num], (PIHANDLER)oldp, (REALPTR)oldr,
						NULL, NULL);
#endif
#endif
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoInterrupt::mask
File:   BO_INTR.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        void BoInterrupt::mask ()

Description
		This function masks the hardware interrupt. No further interrupts
		will be serviced until unmask is called.

 #$%!........................................................................*/

void BoInterrupt::mask ()
{
#ifndef __QNX__
	unsigned char b;

	// disable 8259 hardware interrupt
	if (irq_num < 8)
		{
		_disable ();
		b = inp (0x21);
		(void) outp (0x21, (b | (1u << irq_num))); // set bit for given irq
		}
	else
		{
		short sec_irq = irq_num - 8;
		_disable ();
		// program master
		b = inp (0xa1);
		(void) outp (0xa1, (b | (1u << sec_irq))); // set bit for given irq
		// we don't program slave because somebody else may use it
		}
	_enable ();
#else
	qnx_hint_mask(irq_num,2);
#endif
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoInterrupt::unmask
File:   BO_INTR.CPP
Author: Thomas Buijs
Date:   July, 1994

Synopsis
        void BoInterrupt::unmask ()

Description
		This function unmasks the hardware interrupt.

 #$%!........................................................................*/
#ifdef __QNX__
void BoInterrupt::unmask (pid_t far ( * handler)())
#else
void BoInterrupt::unmask ()
#endif
{
#ifndef __QNX__
	unsigned char b;

	// enable 8259 hardware interrupt
	if (irq_num < 8)
		{
		_disable ();
		b = inp (0x21);
		(void) outp (0x21, (b & ~(1u << irq_num))); // clear bit for given irq
		}
	else
		{
		short sec_irq = irq_num - 8;
		_disable ();
		// program master
		b = inp (0xa1);
		(void) outp (0xa1, (b & ~(1u << sec_irq))); // clear bit for given irq
		// program slave
		b = inp (0x21);
		(void) outp (0x21, (b & ~(1u << 2))); // clear bit for cascade irq
		}
	_enable ();
#else
	/* this attaches the handler and unmasks the interrupt */
	my_iid = qnx_hint_attach(irq_num, handler, my_ds());
#endif
}

// ******************************************************************
// test code.
// ******************************************************************

// Requires a source of hardware interrupts.
// Increments a counter on every interrupt and prints the counter
// on the screen from a loop.

#ifdef DEBUG
// DOS and QNX test code
#include <stdlib.h>
#include <iostream.h>

#ifdef __QNX__
int my_ds;
pid_t handler ();
#else
void interrupt handler ();
#endif

BoInterrupt * volatile aa;
volatile short num = 0;

#ifdef __QNX__
#pragma off( check_stack );
pid_t handler () {
	num++;
	aa->eoi ();
	return 0;
}
#pragma on( check_stack );
#else
void interrupt handler ()
{
	num++;
	aa->eoi ();
}
#endif

main(int argc, char *argv[])
{
	if (argc != 2)
		{
		cout << "Specify interrupt\n";
		exit (1);
		}

#ifdef __QNX__
	BoInterrupt a (atoi (argv[1]));
	aa = &a;

	a.unmask ( handler );
#else
	BoInterrupt a (atoi (argv[1]), handler);
	aa = &a;

	a.unmask ();
#endif

	do 
		{
		cout << num << "\n";
		} while (!kbhit());

	return (0);
}
#endif

#if 0
// Windows test code
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

void interrupt handler ();

BoInterrupt * volatile aa;
volatile short num = 0;

void interrupt handler ()
{
	num++;

	aa->eoi ();
}

main(int argc, char *argv[])
{
	char buf[100];

	if (argc != 2)
		{
		MessageBox (NULL, "", "Specify interrupt", MB_OK|MB_TASKMODAL);
		exit (1);
		}

	BoInterrupt a (atoi (argv[1]), handler);
	aa = &a;

	a.unmask ();

	sprintf (buf, "Counter = %d", num);
	MessageBox (NULL, "", buf, MB_OK|MB_TASKMODAL);
	sprintf (buf, "Counter = %d", num);
	MessageBox (NULL, "", buf, MB_OK|MB_TASKMODAL);
	sprintf (buf, "Counter = %d", num);
	MessageBox (NULL, "", buf, MB_OK|MB_TASKMODAL);
	sprintf (buf, "Counter = %d", num);
	MessageBox (NULL, "", buf, MB_OK|MB_TASKMODAL);
	sprintf (buf, "Counter = %d", num);
	MessageBox (NULL, "", buf, MB_OK|MB_TASKMODAL);

	return (0);
}

#endif
