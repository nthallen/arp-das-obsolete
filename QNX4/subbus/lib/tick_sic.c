/* tick_sic.c Calls resident subbus via Send */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include "subbus.h"
char rcsid_subbus_tick_sic_c[] =
  "$Header$";

/* Under QNX4 always returns 0 */
int set_tps(unsigned int tps) {
  int rv;
  struct sb_tps {
    unsigned char type;
	unsigned int tps;
  } msg;
  
  msg.type = SBMSG_SET_TPS;
  msg.tps = tps;
  do rv = Send(sb_pid, &msg, NULL, sizeof(msg), 0);
	while (rv == -1 && errno == EINTR);
  if (rv == -1) raise(SIG_NOSLIB);
  return(0);
}

/* Under QNX4 always returns 0. Under DOS, returned -1 if queue became
   full.
*/
int tick_sic(void) {
  int rv;
  unsigned char msg = SBMSG_TICK;
  
  do rv = Send(sb_pid, &msg, NULL, 1, 0);
	while (rv == -1 && errno == EINTR);
  if (rv == -1) raise(SIG_NOSLIB);
  return(0);
}

void disarm_sic(void) {
  int rv;
  unsigned char msg = SBMSG_DIS_TICK;
  
  do rv = Send(sb_pid, &msg, NULL, 1, 0);
	while (rv == -1 && errno == EINTR);
  if (rv == -1) raise(SIG_NOSLIB);
}
