/* find_DG() attempts to locate the DG on the local node.
   It will try MAX_RETRIES times with 3 seconds between
   attempts. If DG cannot be found after MAX_RETRIES,
   nl_error(3,...) is called. find_DG() returns the
   pid of the DG or -1 on error (if it returns).
 * $Log$
 * Revision 1.1  1992/09/02  13:26:38  nort
 * Initial revision
 *
*/
#include <sys/types.h>
#include <unistd.h>
#include "nortlib.h"
#include "dbr.h"
#include "company.h"
static char rcsid[] = "$Id$";

static pid_t dg_tid = -1;

pid_t find_DG(void) {
  if (dg_tid == -1)
	dg_tid = nl_find_name(getnid(), COMPANY "/" DG_NAME);
  return(dg_tid);
}
