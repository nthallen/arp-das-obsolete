/* find_CC() attempts to locate CmdCtrl on the local node.
If CC cannot be found and dg_ok is non-zero,
   find_DG() will be called. If DG cannot be found
   nl_error(3,...) is called. find_CC() returns the
   pid of the CC or -1 on error (if it returns).
 * $Log$
 * Revision 1.4  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.3  1992/10/18  19:14:37  nort
 * Removed unused variables
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
#include <sys/name.h>
#include "nortlib.h"
#include "cmdctrl.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

pid_t find_CC(int dg_ok) {
  static pid_t cc_tid = -1;

  if (cc_tid == -1) {
	cc_tid = qnx_name_locate(getnid(), nl_make_name(CMD_CTRL), 0, 0);
	if (cc_tid == -1 && dg_ok) return(find_DG());
  }
  if (cc_tid == -1 && nl_response)
	nl_error(nl_response, "Unable to locate CmdCtrl");
  return(cc_tid);
}
