/* eir_reset.c
 * $Log$
 */
#include <sys/proxy.h>
#include "globmsg.h"
#include "nortlib.h"
#include "timerbd.h"
static char rcsid[] = "$Id$";

int EIR_reset(int EIR) {
  struct tmrbdmsg rqst;
  int rv;
  
  rqst.op_rtn = TMR_RESET;
  rqst.timer = TMR_NONE;
  rqst.eir = EIR;
  rv = send_Tmr(&rqst);
  if (nl_response) {
	switch (rv) {
	  case DAS_BUSY:
		nl_error(nl_response, "EIR owned by someone else");
		break;
	  case DAS_UNKN:
		nl_error(nl_response, "Invalid EIR number");
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
