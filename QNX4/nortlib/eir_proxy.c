/* eir_proxy.c contains EIR_proxy().
 * $Log$
 * Revision 1.1  1993/02/18  02:28:57  nort
 * Initial revision
 *
 */
#include <sys/kernel.h>
#include "globmsg.h"
#include "nortlib.h"
#include "timerbd.h"
#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

int EIR_proxy(int EIR, unsigned char msg) {
  struct tmrbdmsg rqst;
  int rv;
  
  rqst.op_rtn = TMR_SET;
  rqst.timer = TMR_NONE;
  rqst.eir = EIR;
  rqst.action = TMR_PROXY;
  rqst.u.proxy = nl_make_proxy(&msg, 1);
  if (rqst.u.proxy == -1) rv = -1;
  else rv = send_Tmr(&rqst);
  if (nl_response) {
	switch (rv) {
	  case DAS_OK:
	  case -1:
		break;
	  case DAS_BUSY:
		nl_error(nl_response, "EIR %d already attached", EIR);
		break;
	  case DAS_UNKN:
		nl_error(nl_response, "Invalid EIR");
		break;
	  default:
		nl_error(nl_response, "Unexpected Timerbd response");
		break;
	}
  }
  return(rv);
}
