/* find_tmr.c */
#include <unistd.h>
#include "timerbd.h"
#include "nortlib.h"
char rcsid_find_tmr_c[] =
  "$Header$";

static pid_t Tmr_PID = -1;

pid_t find_Tmr(void) {
  if (Tmr_PID == -1)
	Tmr_PID = nl_find_name(getnid(), nl_make_name(TIMERBD_NAME, 0));
  return(Tmr_PID);
}
