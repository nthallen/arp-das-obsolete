/* tmr_proxy.c contains Tmr_proxy(). */
#include <sys/kernel.h>
#include "globmsg.h"
#include "nortlib.h"
#include "timerbd.h"
char rcsid_tmr_proxy_c[] = 
  "$Header$";

int Tmr_proxy(int mode, unsigned short divisor, unsigned char msg) {
  struct tmrbdmsg rqst;
  int rv;
  
  rqst.op_rtn = TMR_SET;
  rqst.timer = TMR_ANY;
  rqst.mode = mode;
  rqst.divisor = divisor;
  rqst.action = TMR_PROXY;
  rqst.u.proxy = nl_make_proxy(&msg, 1);
  if (rqst.u.proxy == -1) rv = -1;
  else rv = send_Tmr(&rqst);
  switch (rv) {
	case DAS_BUSY:
	  if (nl_response) nl_error(nl_response, "No free timers");
	  rv = -1;
	  break;
    default:
	case DAS_UNKN:
	  if (nl_response) nl_error(nl_response, "Unexpected Timerbd response");
	  rv = -1;
	  break;
    case DAS_OK:
	  rv = rqst.timer;
	  break;
    case -1:
	  break;
  }
  return(rv);
}
