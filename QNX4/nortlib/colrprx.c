/* colrprx.c Col_reset_proxy() calls the DG to reset a proxy set
   via Col_set_proxy()
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
/*
=Name Col_reset_proxy(): Reset Collection Proxy
=Subject Data Collection
=Subject Shutdown

=Synopsis

#include "collect.h"
int Col_reset_proxy(unsigned char id);

=Description

  Col_reset_proxy() instructs the TM collection program to
  cease sending to the indicated proxy and detaches the proxy.

  id is the magic number agreed-upon between the process and
  collection for identifying which proxy is designated.

=Returns

  Returns 0 on success. On error, returns -1 unless =nl_response=
  indicates a fatal action.

=SeeAlso

  =Data Collection= functions, =Col_set_proxy=().

=End
*/
