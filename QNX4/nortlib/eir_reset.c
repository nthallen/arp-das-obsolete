/* eir_reset.c
 * $Log$
 * Revision 1.1  1993/02/18  02:28:59  nort
 * Initial revision
 *
 */
#include <sys/proxy.h>
#include "globmsg.h"
#include "nortlib.h"
#include "timerbd.h"
#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

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
