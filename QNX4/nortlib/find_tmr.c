/* find_tmr.c
 * $Log$
 * Revision 1.2  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.1  1993/02/18  02:28:46  nort
 * Initial revision
 *
 */
#include <unistd.h>
#include "timerbd.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static pid_t Tmr_PID = -1;

pid_t find_Tmr(void) {
  if (Tmr_PID == -1)
	Tmr_PID = nl_find_name(getnid(), nl_make_name(TIMERBD_NAME));
  return(Tmr_PID);
}
