/* find_tmr.c
 * $Log$
 */
#include <unistd.h>
#include "timerbd.h"
#include "nortlib.h"
static char rcsid[] = "$Id$";

static pid_t Tmr_PID = -1;

pid_t find_Tmr(void) {
  if (Tmr_PID == -1)
	Tmr_PID = nl_find_name(getnid(), TIMERBD_NAME);
  return(Tmr_PID);
}
