/* find_DG() attempts to locate the DG on the local node.
   If DG cannot be found nl_error(3,...) is called.
   find_DG() returns the
   pid of the DG or -1 on error (if it returns).
*/
#include <sys/types.h>
#include <unistd.h>
#include "nortlib.h"
#include "dbr.h"
#include "company.h"
char rcsid_find_dg_c[] =
  "$Header$";

static pid_t dg_tid = -1;

pid_t find_DG(void) {
  if (dg_tid == -1)
	dg_tid = nl_find_name(getnid(), nl_make_name(DG_NAME, 1));
  return(dg_tid);
}
