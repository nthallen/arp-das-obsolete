/* make_proxy.c nortlib standard function for creating proxies
 * $Log$
 */
#include <sys/proxy.h>
#include "nortlib.h"
static char rcsid[] = "$Id$";

pid_t nl_make_proxy(void *msg, int size) {
  pid_t proxy;
  
  proxy = qnx_proxy_attach(0, msg, size, -1);
  if (proxy == -1 && nl_response)
	nl_error(nl_response, "Unable to create proxy");
  return(proxy);
}
