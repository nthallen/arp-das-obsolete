/* find_DG() attempts to locate the DG on the local node.
   If DG cannot be found nl_error(3,...) is called.
   find_DG() returns the
   pid of the DG or -1 on error (if it returns).
 * $Log$
 * Revision 1.3  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.2  1992/10/18  19:12:04  nort
 * Removed wait loops.
 *
 * Revision 1.1  1992/10/18  19:07:40  nort
 * Initial revision
 *
 * Revision 1.1  1992/09/02  13:26:38  nort
 * Initial revision
 *
*/
#include <sys/types.h>
#include <unistd.h>
#include "nortlib.h"
#include "dbr.h"
#include "company.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static pid_t dg_tid = -1;

pid_t find_DG(void) {
  if (dg_tid == -1)
	dg_tid = nl_find_name(getnid(), nl_make_name(DG_NAME));
  return(dg_tid);
}
