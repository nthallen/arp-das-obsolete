/* repbyte.c Provides trivial reply_byte() function.
 * Always returns 0. Could be augmented to check success...
 * $Log$
 * Revision 1.1  1992/09/02  20:17:18  nort
 * Initial revision
 *
 */
#include <sys/kernel.h>
#include "nortlib.h"
#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

int reply_byte(pid_t sent_tid, unsigned char msg) {
  Reply(sent_tid, &msg, 1);
  return(0);
}
