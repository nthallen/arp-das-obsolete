/* Col_reset_pointer requests that the DG reset a pointer
   previously passed by this process via Col_set_pointer.
 * $Log$
 * Revision 1.1  1992/09/02  13:26:38  nort
 * Initial revision
 *
*/
#include <sys/seginfo.h>
#include <sys/kernel.h>
#include "collect.h"
#include "nortlib.h"
#include "globmsg.h"
#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

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

