/* senddasc.c defines send_dascmd()
 * send_dascmd() returns 0 on success. Failure to locate CC (or DG)
 * is handled according to nl_response rules. Valid error returns
 * from CC (or DG) are returned to the calling program.
 * $Log$
 */
#include "cmdctrl.h"
#include "nortlib.h"
static char rcsid[] = "$Id$";

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
