/* handler.c contains the interrupt handler which should be able to
   safely call the few library routines which we need to call.

   This module must be compiled -Wc,-s since the stack will be
   different from the usually process stack, so stack checking
   doesn't work.

   As a general rule, no library routines should be called from
   this module. The exceptions I will use are:
     read_subbus      to determine the source of the interrupt
     write_subbus     to send EOI to the system board
*/
#include "timerbd.h"
#include "tmrdrvr.h"
#include "subbus.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

/* mvecs() sets or resets the mask register for a particular EIR */
static void mvecs(int vector, int clear) {
  int mask, imr_addr, imr;

  switch (vector & 0xF8) {
	case 0:
	  imr_addr = epic1 + ocw1;
	  if (vector == 6 || vector == 7) return;
	  break;
	case 8:
	  imr_addr = epic2 + ocw1;
	  break;
	case 16:
	  imr_addr = epic3 + ocw1;
	  break;
	default:
	  return;
  }
  mask = 1 << (vector & 7);
  imr = sbw(imr_addr) & 0xFF;
  if (clear) imr &= ~mask;
  else imr |= mask;
  sbwr(imr_addr, imr);
}

void EIR_mask(int vector) { mvecs(vector, 0); }
void EIR_unmask(int vector) { mvecs(vector, 1); }

static void send_eoi(unsigned short vector) {
  if (vector < N_EIRS) {
	switch (vector/8) {
	  case 0:
		sbwr(epic1+ocw2, 0x60 | vector);
		break;
	  case 1:
		sbwr(epic2+ocw2, 0x60 | (vector & 7));
		sbwr(epic1+ocw2, 0x66);
		break;
	  case 2:
		/* Because the EPIC1 is in SFNM, it should never require
		   and EOI for interrupts off EPIC3. */
		sbwr(epic3+ocw2, 0x60 | (vector & 7));
		break;
	}
  }
}

/* tmr_handler returns a proxy value that should be triggered.
   A return of zero indicates no action should be taken.
*/
pid_t far tmr_handler(void) {
  unsigned short vector;
  pid_t proxy = 0;

  sbw(TMR_INTA);
  vector = sbw(TMR_INTA) & 0xFF;
  if (vector < N_EIRS) {
    if ((vector & 7) != 7
		|| (sbw(TMR_ISR_ADDR(vector)) & 0x80) != 0) {
      switch (eirvs[vector].action) {
        case TMR_PROXY:
		  proxy = eirvs[vector].u.proxy;
          break;
        case TMR_NOP:
		default:
          EIR_mask(vector);
          break;
      }
    }
	send_eoi(vector);
  }
  return(proxy);
}

