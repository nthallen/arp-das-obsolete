/* nl_find_name() provides a general-purpose approach to finding other
   processes.
   It will try MAX_RETRIES times with 3 seconds between
   attempts. If name cannot be found after MAX_RETRIES,
   nl_error(nl_response,...) is called. nl_find_name() returns the
   pid or -1 on error (if it returns).
 * $Log$
*/
#include <sys/types.h>
#include <sys/name.h>
#include <unistd.h>
#include "nortlib.h"
static char rcsid[] = "$Id$";

#define MAX_RETRIES 2

pid_t nl_find_name(nid_t node, char *name) {
  int i;
  pid_t pid;

  for (i = 0; ;) {
	pid = qnx_name_locate(node, name, 0, 0);
	if (pid != -1 || ++i >= MAX_RETRIES) break;
	sleep(3);
  }
  if (pid == -1 && nl_response)
	nl_error(nl_response, "Unable to locate %s", name);
  return(pid);
}
