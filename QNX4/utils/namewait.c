/* namewait.c Utility to wait until a specified name is registered.
 * Strategy is to modify priority to be very low and loop, attempting
 * to locate the name on the specified node. I should also provide
 * an optional timeout.
 *
 *   namewait [-n node] [-t seconds] [-p pid] name
 *
 * $Log$
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sched.h>
#include <sys/name.h>
#include <sys/kernel.h>
#include <sys/psinfo.h>
#include <sys/seginfo.h>
#include <sys/vc.h>
#include <unistd.h>
#include <time.h>
#include "nortlib.h"
static char rcsid[] = "$Id$";

#ifdef __USAGE
%C	[-n n] [-t s] [-p pid] name
	Wait until name is registered
	Options:
	-n n    Look for name on node n. (default is current node)
	-n 0    Look for name on all nodes.
	-t s    Fail if name is not registered within s seconds
	-p pid  Fail if specified process terminates
#endif

int main(int argc, char **argv) {
  int c, oldpri;
  nid_t node;
  time_t timeout, t0;
  pid_t pid, spid, procid;
  struct _psinfo psdata;
  
  node = getnid();
  spid = 0;
  timeout = 0;
  opterr = 0; /* Disable getopt's error messages */
  while ( (c = getopt(argc, argv, "n:t:p:")) != -1) {
	switch (c) {
	  case 'n':
		node = atoi(optarg);
		break;
	  case 't':
		timeout = atoi(optarg);
		break;
	  case 'p':
		spid = atoi(optarg);
		break;
	  case '?': nl_error(3, "Illegal option -%c", optopt);
	}
  }
  if (optind >= argc)
	nl_error(3, "Must specify name");
  if (spid != 0) {
	procid = qnx_vc_attach(node, PROC_PID, 0, 0);
	if (procid == -1)
	  nl_error(3, "Unable to attach to PROC on node %d", node);
  }
  oldpri = qnx_scheduler(0, 0, -1, 1, 0);
  if (timeout > 0) t0 = time(NULL);
  for (;;) {
	pid = qnx_name_locate(node, argv[optind], 0, NULL);
	if (pid > 0) break;
	if (spid != 0 && (qnx_psinfo(procid, spid, &psdata, 0, NULL) == -1
			|| psdata.pid != spid)) break;
	if (timeout > 0 && timeout <= time(NULL) - t0) break;
  }
  qnx_scheduler(0, 0, -1, oldpri, 0);
  if (pid > 0) {
	if (spid != 0 && pid != spid)
	  nl_error(3, "Name registered by different process");
  } else {
	if (spid != 0 && (qnx_psinfo(procid, spid, &psdata, 0, NULL) == -1
			|| psdata.pid != spid))
	  nl_error(3, "Specified Process Terminated");
	if (timeout > 0 && timeout <= time(NULL) - t0)
	  nl_error(3, "Timeout without locating name %s", argv[optind]);
  }
  return(0);
}
