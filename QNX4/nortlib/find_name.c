/* nl_find_name() provides a general-purpose approach to finding other
   processes.
   If name cannot be found
   nl_error(nl_response,...) is called. nl_find_name() returns the
   pid or -1 on error (if it returns).
 * $Log$
 * Revision 1.1  1992/10/18  19:05:53  nort
 * Initial revision
 *
*/
#include <sys/types.h>
#include <sys/name.h>
#include <unistd.h>
#include "nortlib.h"
static char rcsid[] = "$Id$";

pid_t nl_find_name(nid_t node, char *name) {
  int i;
  pid_t pid;

  pid = qnx_name_locate(node, name, 0, 0);
  if (pid == -1 && nl_response)
	nl_error(nl_response, "Unable to locate %s", name);
  return(pid);
}
