/* send_CC sends the message to CmdCtrl returning the reply in the
   same package. Returns zero if the send operation is a success.
   The calling process is responsible for handling CmdCtrl's return
   codes.
*/
#include <sys/types.h>
#include <sys/kernel.h>
#include <errno.h>
#include "nortlib.h"
char rcsid_send_cc_c[] =
  "$Header$";

int send_CC(void *msg, int size, int dg_ok) {
  int rv;
  pid_t ccpid;

  if ((ccpid = find_CC(dg_ok)) == -1) return(-1);
  rv = Send(ccpid, msg, msg, size, size);
  if (rv == -1 && nl_response)
	nl_error(nl_response, "Unable to Send to CMDCTRL: pid=%d", ccpid);
  return(rv);
}
