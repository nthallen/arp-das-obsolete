/* pic.c handles initialization of the 8259 Programmable Interrupt Controllers.
 * $Log$
 * Revision 1.2  1995/06/01  01:15:56  nort
 * *** empty log message ***
 *
 * Revision 1.1  1992/10/26  16:17:31  nort
 * Initial revision
 *
 * Revision 1.1  1992/09/24  20:50:28  nort
 * Initial revision
 *
 
   Modified for QNX January 21, 1991
   Modified from Series IV pic.asm to IBM PC pic.c February 12, 1987
   Modified 5-June-86 by NTA for v2.20
*/
#include "tmrdrvr.h"
#include "subbus.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static struct {
  int addr;
  int data;
} init_array[] = {
/*-------------------------------------------------------------------*/
/* initialize the second external slave:  epic3						 */
/*-------------------------------------------------------------------*/
  epic3 + icw1, 0x11,	/* Edge triggered, cascade mode */
  epic3 + icw2, 16,		/* Starting with IR #16 (not vectored) */
  epic3 + icw3, 0x7,	/* We are IR7 to the master */
  epic3 + icw4, 0x1,	/* FNM, unbuffered, NEOI */
  epic3 + ocw1, 0xFF,	/* All disabled. */
  epic3 + ocw3, 0xB,	/* Prepare to read ISR */
/*-------------------------------------------------------------------*/
/* initialize the first external slave:	 epic2						 */
/*-------------------------------------------------------------------*/
  epic2 + icw1, 0x11,	/* Edge triggered, cascade mode */
  epic2 + icw2, 8,		/* Starting with IR #8 (not vectored) */
  epic2 + icw3, 0x6,	/* We are IR6 to the master */
  epic2 + icw4, 0x1,	/* FNM, unbuffered, NEOI */
  epic2 + ocw1, 0xFF,	/* All disabled */
  epic2 + ocw3, 0xB,	/* Prepare to read ISR */
/*-------------------------------------------------------------------*/
/* initialize the external master:	epic1							 */
/*-------------------------------------------------------------------*/
  epic1 + icw1, 0x11,	/* Edge triggered, cascade mode */
  epic1 + icw2, 0,		/* Starting at 0 (not vectored) */
  epic1 + icw3, 0xC0,	/* Slaves on IRs 6 & 7 */
  epic1 + icw4, 0x11,	/* SFNM, unbuffered, NEOI */
  epic1 + ocw1, 0x3F,	/* All disabled but the slaves */
  epic1 + ocw3, 0xB,	/* Prepare to read ISR */
  0, 0
};

/*-------------------------------------------------------------------*/
/* picinit is the routine to initialize all of the 8259				 */
/* Programmable Interrupt Controllers.								 */
/*-------------------------------------------------------------------*/
int picinit(void) {
  int i;

  for (i = 0; init_array[i].addr != 0; i++)
	if (sbwra(init_array[i].addr, init_array[i].data) == 0) return(1);
  return(0);
}
