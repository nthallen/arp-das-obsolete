/* Col_set_proxy creates a proxy and passes it to the DG.
 * $Log$
 * Revision 1.1  1992/09/02  13:26:38  nort
 * Initial revision
 *
*/
#include <sys/proxy.h>
#include "collect.h"
#include "nortlib.h"
#include "globmsg.h"
static char rcsid[] = "$Id$";

pid_t Col_set_proxy(unsigned char id, unsigned char msg) {
  struct colmsg c;
  int rv;
  
  c.type = COL_SET_PROXY;
  c.id = id;
  c.u.proxy = nl_make_proxy(&msg, 1);
  if (c.u.proxy == -1) rv = -1;
  else if ((rv = send_DG(&c, sizeof(struct colmsg))) == 0) {
	rv = c.u.proxy;
	if (c.type != DAS_OK) {
	  rv = -1;
	  if (nl_response)
		nl_error(nl_response, "Error from DG setting proxy");
	  qnx_proxy_detach(c.u.proxy);
	}
  }
  return(rv);
}
