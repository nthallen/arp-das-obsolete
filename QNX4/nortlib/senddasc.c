/* senddasc.c defines send_dascmd()
 * send_dascmd() returns 0 on success. Failure to locate CC (or DG)
 * is handled according to nl_response rules. Valid error returns
 * from CC (or DG) are returned to the calling program.
 */
#include <sys/types.h>
#include "cmdctrl.h"
#include "nortlib.h"
char rcsid_senddasc_c[] =
  "$Header$";

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

/*
=Name send_dascmd(): Send a DASCMD
=Subject Client/Server
=Subject Data Collection

=Synopsis
#include "nortlib.h"

int send_dascmd(int type, int value, int dg_ok);

=Description

Sends the specified DASCMD to CmdCtrl or the DG. DASCMD types
are defined in <b>globmsg.h</b>. The dg_ok argument specifies
whether it is reasonable to send the command to the DG if
a CmdCtrl cannot be found. This should generally be set to a
non-zero value if the command is in fact intended for the DG,
such as DCT_TM or DCT_QUIT.

=Returns

send_dascmd() returns 0 on success. Failure to locate CC (or DG)
is handled according to nl_response rules. Valid error returns
from CC (or DG) are returned to the calling program.

=SeeAlso

=DigSelect=(), =Client/Server= functions.

=End
*/
