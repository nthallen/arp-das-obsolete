/* senddasc.c defines send_dascmd()
 * send_dascmd() returns 0 on success. Failure to locate CC (or DG)
 * is handled according to nl_response rules. Valid error returns
 * from CC (or DG) are returned to the calling program.
 * $Log$
 * Revision 1.2  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.1  1992/09/02  13:26:38  nort
 * Initial revision
 *
 */
#include <sys/types.h>
#include "cmdctrl.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int send_dascmd(int type, int value, int dg_ok) {
  dasc_msg_type dm;
  int rv;
  
  dm.dasc_type = DASCMD;
  dm.dascmd.type = type;
  dm.dascmd.val = value;
  rv = send_CC(&dm, sizeof(dm), dg_ok);
  if (rv == 0) rv = dm.dasc_type;
  return(rv);
}
