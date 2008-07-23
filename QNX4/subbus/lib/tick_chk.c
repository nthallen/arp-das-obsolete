/* tick_chk.c Calls resident subbus via Send */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include "subbus.h"
char rcsid_subbus_tick_chk_c[] =
  "$Header$";

/* Always returns 0 under QNX4 */
int tick_check(char id, unsigned int secs) {
  int rv;
  struct sb_tchk {
	unsigned char type;
	unsigned char id;
	unsigned int secs;
  } msg;

  msg.type = SBMSG_TICK_CHK;
  msg.id = id;
  msg.secs = secs;
  do rv = Send(sb_pid, &msg, NULL, sizeof(msg), 0);
	while (rv == -1 && errno == EINTR);
  if (rv == -1) raise(SIG_NOSLIB);
  return(0);
}
