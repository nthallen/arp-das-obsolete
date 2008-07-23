/* reboot.c Calls resident subbus via Send */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include "subbus.h"
char rcsid_subbus_reboot_c[] =
  "$Header$";

void reboot(unsigned char critstat) {
  int rv;
  struct sb_reboot {
    unsigned char type;
	unsigned char critstat;
  } msg;

  msg.type = SBMSG_REBOOT;
  msg.critstat = critstat;  
  do rv = Send(sb_pid, &msg, NULL, sizeof(msg), 0);
	while (rv == -1 && errno == EINTR);
  if (rv == -1) raise(SIG_NOSLIB);
}
