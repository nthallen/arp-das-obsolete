/* Col_set_proxy creates a proxy and passes it to the DG. */
#include <sys/proxy.h>
#include "collect.h"
#include "nortlib.h"
#include "globmsg.h"
char rcsid_colsprx_c[] =
  "$Header$";

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
/*
=Name Col_set_proxy(): Setup Synchronization with Collection
=Subject Data Collection
=Subject Startup

=Synopsis

#include "collect.h"
pid_t Col_set_proxy(unsigned char id, unsigned char msg);

=Description

  <P>Col_set_proxy() provides a mechanism for synchronizing a
  process with data collection, which is often useful for
  low-level control functions such as scanning. This function
  creates a one-byte-long proxy message and passes it to
  collection to be trigger at an agreed-upon rate. The actual
  triggering of the proxy must be explicitly coded into the
  collection process, but there is an established TMC syntax to
  facilitate the registration process.</P>
  
  <P>id is a magic number agreed-upon between the process and
  collection for identifying which proxy is designated. msg is
  the one-byte contents of the proxy which will be created.</P>
  
  <P>On termination, =Col_reset_proxy=() may be called to provide an
  orderly shutdown.</P>

=Returns

  <P>Col_set_proxy() returns the proxy's pid if successful.
  Otherwise it returns -1 unless =nl_response= indicates a fatal
  response.</P>

=SeeAlso

  =Data Collection= functions, =Col_reset_proxy=().

=End
*/
