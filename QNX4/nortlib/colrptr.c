/* Col_reset_pointer requests that the DG reset a pointer
   previously passed by this process via Col_set_pointer.
 * $Log$
*/
#include <sys/seginfo.h>
#include <sys/kernel.h>
#include "collect.h"
#include "nortlib.h"
#include "globmsg.h"
static char rcsid[] = "$Id$";

int Col_reset_pointer(unsigned char id) {
  struct colmsg c;
  pid_t dgpid;
  int rv;
  
  c.type = COL_RESET_POINTER;
  c.id = id;
  dgpid = find_DG();
  if ((rv = send_DG(&c, sizeof(struct colmsg))) == 0
	  && c.type != DAS_OK) {
	rv = -1;
	if (nl_response)
	  nl_error(nl_response, "Error from DG seting pointer");
  }
  return(rv);
}

