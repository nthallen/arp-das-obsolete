/* nl_find_name() provides a general-purpose approach to finding other
   processes.
   If name cannot be found
   nl_error(nl_response,...) is called. nl_find_name() returns the
   pid or -1 on error (if it returns).
 * $Log$
 * Revision 1.4  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.3  1992/10/18  19:14:37  nort
 * Removed unused variables
 *
 * Revision 1.2  1992/10/18  19:06:58  nort
 * Remove retries, count on namewait to wait as necessary.
 *
 * Revision 1.1  1992/10/18  19:05:53  nort
 * Initial revision
 *
*/
#include <sys/types.h>
#include <sys/name.h>
#include <unistd.h>
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

pid_t nl_find_name(nid_t node, char *name) {
  pid_t pid;

  pid = qnx_name_locate(node, name, 0, 0);
  if (pid == -1 && nl_response)
	nl_error(nl_response, "Unable to locate %s", name);
  return(pid);
}
