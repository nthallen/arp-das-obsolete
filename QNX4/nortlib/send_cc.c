/* send_CC sends the message to CmdCtrl returning the reply in the
   same package. Returns zero if the send operation is a success.
   The calling process is responsible for handling CmdCtrl's return
   codes.
 * $Log$
 * Revision 1.1  1992/09/02  13:26:38  nort
 * Initial revision
 *
*/
#include <sys/types.h>
#include <sys/kernel.h>
#include <errno.h>
#include "nortlib.h"
static char rcsid[] = "$Id$";

int send_CC(void *msg, int size, int dg_ok) {
  int rv;
  pid_t ccpid;

  if ((ccpid = find_CC(dg_ok)) == -1) return(-1);
  rv = Send(ccpid, msg, msg, size, size);
  if (rv == -1 && nl_response)
	nl_error(nl_response, "Unable to Send to CMDCTRL: pid=%d", ccpid);
  return(rv);
}
