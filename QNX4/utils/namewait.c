/* namewait.c Utility to wait until a specified name is registered.
 * Strategy is to modify priority to be very low and loop, attempting
 * to locate the name on the specified node. I should also provide
 * an optional timeout.
 *
 *   namewait [-n node] [-t seconds] [-p pid] name
 *
 * $Log$
 * Revision 1.2  1993/09/15  19:22:42  nort
 * Modifications for Experiment name expansion
 *
 * Revision 1.1  1993/09/15  19:02:12  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
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
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

#ifdef __USAGE
%C	[-n n] [-t s] [-p pid] [-x] name
	Wait until name is registered
	Options:
	-g      Make name global. (effective only if name is expanded
	        using the Experiment environment variable.)
	-n n    Look for name on node n. (default is current node)
	-n 0    Look for name on all nodes. (If name is expanded, it
	        will be made global also.)
	-t s    Fail if name is not registered within s seconds
	-p pid  Fail if specified process terminates
	-x      Take given name literally: don't expand using Experiment
	-N      Register the global name 'namewait' while waiting
#endif

int main(int argc, char **argv) {
  int c, oldpri, name_id;
  int expand_name = 1, is_global = 0, register_name = 0;
  nid_t node;
  char *name;
  time_t timeout = -1, t0;
  pid_t pid, spid = 0, procid;
  struct _psinfo psdata;
  
  node = getnid();
  opterr = 0; /* Disable getopt's error messages */
  while ( (c = getopt(argc, argv, "n:t:p:xgN")) != -1) {
	switch (c) {
	  case 'g': is_global = 1; break;
	  case 'n': node = atoi(optarg); break;
	  case 't': timeout = atoi(optarg); break;
	  case 'p': spid = atoi(optarg); break;
	  case 'x': expand_name = 0; break;
	  case 'N': register_name = 1; break;
	  case '?': nl_error(3, "Illegal option -%c", optopt);
	}
  }
  if (optind >= argc)
	nl_error(3, "Must specify name");

  /* Register name */
  if (register_name) {
	name_id = qnx_name_attach(0, nl_make_name("namewait", 1));
	if (name_id == -1)
	  nl_error(3, "Unable to attach global name");
  }

  name = argv[optind];
  if (node == 0) is_global = 1;
  if (expand_name && strchr(name, '/') == NULL)
	name = nl_make_name(name, is_global);
  if (spid != 0) {
	procid = qnx_vc_attach(node, PROC_PID, 0, 0);
	if (procid == -1)
	  nl_error(3, "Unable to attach to PROC on node %d", node);
  }
  oldpri = qnx_scheduler(0, 0, -1, 1, 0);
  if (timeout > 0) t0 = time(NULL);
  for (;;) {
	pid = qnx_name_locate(node, name, 0, NULL);
	if (pid > 0) break;
	if (spid != 0 && (qnx_psinfo(procid, spid, &psdata, 0, NULL) == -1
			|| psdata.pid != spid)) break;
	if (timeout == 0 ||
		(timeout > 0 && timeout <= time(NULL) - t0)) break;
  }
  qnx_scheduler(0, 0, -1, oldpri, 0);
  
  /* detach name */
  if (register_name) qnx_name_detach(0, name_id);
  
  if (pid > 0) {
	if (spid != 0 && pid != spid)
	  nl_error(3, "Name registered by different process");
  } else {
	if (spid != 0 && (qnx_psinfo(procid, spid, &psdata, 0, NULL) == -1
			|| psdata.pid != spid))
	  nl_error(3, "Specified Process Terminated");
	if (timeout == 0 ||
		(timeout > 0 && timeout <= time(NULL) - t0))
	  nl_error(3, "Timeout without locating name %s", name);
  }
  return(0);
}
