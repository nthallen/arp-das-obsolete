/* colrprx.c Col_reset_proxy() calls the DG to reset a proxy set
   via Col_set_proxy()
 * $Log$
 * Revision 1.2  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.1  1992/09/02  13:26:38  nort
 * Initial revision
 *
*/
#include <sys/proxy.h>
#include "collect.h"
#include "nortlib.h"
#include "globmsg.h"
char rcsid_colrprx_c[] =
  "$Header$";

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
