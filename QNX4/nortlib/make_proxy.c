/* make_proxy.c nortlib standard function for creating proxies
 * $Log$
 * Revision 1.1  1993/02/18  02:28:54  nort
 * Initial revision
 *
 */
#include <sys/proxy.h>
#include "nortlib.h"
#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

pid_t nl_make_proxy(void *msg, int size) {
  pid_t proxy;
  
  proxy = qnx_proxy_attach(0, msg, size, -1);
  if (proxy == -1 && nl_response)
	nl_error(nl_response, "Unable to create proxy");
  return(proxy);
}
