/* tick_chk.c Calls resident subbus via Send */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include "subbus.h"
char rcsid_subbus_nmi_c[] =
  "$Header$";

void disable_nmi(void) {
  int rv;
  unsigned char msg = SBMSG_DIS_NMI;
  
  do rv = Send(sb_pid, &msg, NULL, 1, 0);
	while (rv == -1 && errno == EINTR);
  if (rv == -1) raise(SIG_NOSLIB);
}

#ifdef __WATCOMC__
  #pragma off (unreferenced)
#endif

void enable_nmi(void (far *func)(void)) {

#ifdef __WATCOMC__
  #pragma on (unreferenced)
#endif

  int rv;
  unsigned char msg = SBMSG_ENA_NMI;

  do rv = Send(sb_pid, &msg, NULL, 1, 0);
	while (rv == -1 && errno == EINTR);
  if (rv == -1) raise(SIG_NOSLIB);
}
