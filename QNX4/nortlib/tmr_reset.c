/* tmr_reset.c
 * $Log$
 */
#include <sys/proxy.h>
#include "globmsg.h"
#include "nortlib.h"
#include "timerbd.h"
static char rcsid[] = "$Id$";

int Tmr_reset(int timer) {
  struct tmrbdmsg rqst;
  int rv;
  
  rqst.op_rtn = TMR_RESET;
  rqst.timer = timer;
  rv = send_Tmr(&rqst);
  if (nl_response) {
	switch (rv) {
	  case DAS_BUSY:
		nl_error(nl_response, "Timer owned by someone else");
		break;
	  case DAS_UNKN:
		nl_error(nl_response, "Invalid timer number");
		break;
	  case -1:
	  case DAS_OK:
		break;
	}
  }
  if (rv == DAS_OK && rqst.action == TMR_PROXY)
	qnx_proxy_detach(rqst.u.proxy);
  return(rv);
}

