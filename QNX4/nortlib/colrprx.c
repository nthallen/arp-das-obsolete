/* colrprx.c Col_reset_proxy() calls the DG to reset a proxy set
   via Col_set_proxy()
 * $Log$
*/
#include <sys/proxy.h>
#include "collect.h"
#include "nortlib.h"
#include "globmsg.h"
static char rcsid[] = "$Id$";

/* resets a proxy set with Col_set_proxy(). Even detaches it properly.
*/
int Col_reset_proxy(unsigned char id) {
  struct colmsg c;
  int rv;
  
  c.type = COL_RESET_PROXY;
  c.id = id;
  if ((rv = send_DG(&c, sizeof(struct colmsg))) == 0) {
	if (c.type == DAS_OK) qnx_proxy_detach(c.u.proxy);
	else {
	  if (nl_response)
		nl_error(nl_response, "Error from DG resetting proxy");
	  rv = -1;
	}
  }
  return(rv);
}
