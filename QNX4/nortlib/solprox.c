/* solprox.c Sends soldrv proxy requests */
#include <sys/proxy.h>
#include "globmsg.h"
#include "nortlib.h"
#include "soldrv.h"
char rcsid_solprox_c[] =
  "$Header$";

int Soldrv_set_proxy(unsigned char selector, unsigned char ID,
					void *msg, int size) {
  struct {
	msg_hdr_type selector;
	proxy_reg_type p;
  } smsg;

  smsg.selector = selector;
  smsg.p.set_or_reset = SOL_SET_PROXY;
  smsg.p.proxy_id = ID;
  smsg.p.proxy_pid = nl_make_proxy(msg, size);
  if (smsg.p.proxy_pid == -1
	  || send_CC(&smsg, sizeof(smsg), 0) == -1)
	return(-1);
  if (smsg.selector != DAS_OK && nl_response)
	nl_error(nl_response, "Error setting soldrv proxy");
  return(smsg.selector);
}

int Soldrv_reset_proxy(unsigned char selector, unsigned char ID) {
  struct {
	unsigned char selector;
	proxy_reg_type p;
  } smsg;

  smsg.selector = selector;
  smsg.p.set_or_reset = SOL_RESET_PROXY;
  smsg.p.proxy_id = ID;
  if (send_CC(&smsg, sizeof(smsg), 0) == -1)
	return(-1);
  if (smsg.selector == DAS_OK)
	qnx_proxy_detach(smsg.p.proxy_pid);
  else if (nl_response)
	nl_error(nl_response, "Error resetting soldrv proxy");
  return(smsg.selector);
}
