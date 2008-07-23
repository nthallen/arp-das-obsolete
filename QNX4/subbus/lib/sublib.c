/* sublib.c defines the sbfs data structure */
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/seginfo.h>
#include <unistd.h>
#include "subbus.h"
char rcsid_subbus_sublib_c[] =
  "$Header$";

#define COMPANY "huarp"
#define LOCAL_SYMNAME(x) COMPANY "/" x

pid_t sb_pid = 0;

struct sbf sbfs = {
  0, 0, 0,
  (unsigned int (far *)(unsigned int)) sbsnload,
  (int (far *)(unsigned int, unsigned int)) sbsnload,
  (int (far *)(unsigned int, unsigned int far *)) sbsnload,
  (void (far *)(int value)) sbsnload,
  (unsigned char (far *)(unsigned int)) sbsnload,
  (void (far *)(unsigned int, unsigned char)) sbsnload,
  (unsigned int (far *)(void)) sbsnload,
  (void (far *)(int)) sbsnload,
  (unsigned char (far *)(void)) sbsnload,
  (unsigned char (far *)(void)) sbsnload,
  (unsigned char (far *)(void)) sbsnload,
  (short int (far *)(short int)) sbsnload
};

/* Returns the library subfunction code on success or zero on failure */
int load_subbus(void) {
  unsigned char msg;
  unsigned int rv;
  
  sb_pid = qnx_name_locate(getnid(), LOCAL_SYMNAME("subbus"), 0, NULL);
  if (sb_pid != -1) {
	qnx_segment_arm(sb_pid, -1, 0);
	msg = SBMSG_LOAD;
	do rv = Send(sb_pid, &msg, &sbfs, sizeof(msg), sizeof(sbfs));
	  while (rv == -1 && errno == EINTR);
	if (rv != -1) {
	  /* Check version */
	  if (subbus_version >= 0x300 && sbfs.subfunction != 0) {
		return(sbfs.subfunction);
	  }
	}
  }
  errno = ELIBACC;
  return(0);
}
