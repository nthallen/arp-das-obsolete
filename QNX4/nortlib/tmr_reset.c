/* tmr_reset.c
 * $Log$
 * Revision 1.1  1993/02/18  02:29:05  nort
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

