/* send_DG sends the message to the DG returning the reply in the
   same package. Returns zero if the send operation is a success.
   The calling process is responsible for handling the DG's return
   codes.
 * $Log$
*/
#include <sys/types.h>
#include <sys/kernel.h>
#include <errno.h>
#include "nortlib.h"
static char rcsid[] = "$Id$";

int send_DG(void *msg, int size) {
  int rv;
  pid_t dgpid;

  if ((dgpid = find_DG()) == -1) return(-1);
  while ((rv = Send(dgpid, msg, msg, size, size)) == -1
			&& errno == EINTR);
  if (rv == -1 && nl_response)
	nl_error(nl_response, "Unable to Send to DG");
  return(rv);
}
