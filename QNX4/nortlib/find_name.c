/* nl_find_name() provides a general-purpose approach to finding other
   processes.
   If name cannot be found
   nl_error(nl_response,...) is called. nl_find_name() returns the
   pid or -1 on error (if it returns).
*/
#include <sys/types.h>
#include <sys/name.h>
#include <unistd.h>
#include "nortlib.h"
char rcsid_find_name_c[] =
  "$Header$";

pid_t nl_find_name(nid_t node, const char *name) {
  pid_t pid;

  if (name == 0) pid = -1;
  else {
	pid = qnx_name_locate(node, name, 0, 0);
	if (pid == -1 && nl_response)
	  nl_error(nl_response, "Unable to locate %s", name);
  }
  return(pid);
}
