/* sb_name.c Calls resident subbus via Send */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include "subbus.h"
char rcsid_subbus_sb_name_c[] =
  "$Header$";

#define SBNAMESIZE 80
char *get_subbus_name(void) {
  int rv;
  unsigned char msg;
  static char name[SBNAMESIZE];

  msg = SBMSG_GET_NAME;
  do rv = Send(sb_pid, &msg, name, sizeof(msg), SBNAMESIZE);
	while (rv == -1 && errno == EINTR);
  if (rv == -1) raise(SIG_NOSLIB);
  name[SBNAMESIZE-1] = '\0';
  return(name);
}
