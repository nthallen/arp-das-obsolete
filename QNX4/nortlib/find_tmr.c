/* find_tmr.c
 * $Log$
 * Revision 1.1  1993/02/18  02:28:46  nort
 * Initial revision
 *
 */
#include <unistd.h>
#include "timerbd.h"
#include "nortlib.h"
#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

static pid_t Tmr_PID = -1;

pid_t find_Tmr(void) {
  if (Tmr_PID == -1)
	Tmr_PID = nl_find_name(getnid(), TIMERBD_NAME);
  return(Tmr_PID);
}
