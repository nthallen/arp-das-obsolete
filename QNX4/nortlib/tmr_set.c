/* tmr_set.c
 * $Log$
 */
#include "globmsg.h"
#include "nortlib.h"
#include "timerbd.h"
static char rcsid[] = "$Id$";

int Tmr_set(int timer, int mode, unsigned short divisor) {
  struct tmrbdmsg rqst;
  int rv;
  
  rqst.op_rtn = TMR_SET;
  rqst.timer = timer;
  rqst.mode = mode;
  rqst.divisor = divisor;
  rqst.action = TMR_QUERY;
  rv = send_Tmr(&rqst);
  if (nl_response) {
	switch (rv) {
	  case DAS_BUSY:
		nl_error(nl_response, "Timer owned by someone else");
		break;
	  case DAS_UNKN:
		nl_error(nl_response, "Invalid timer number");
		break;
	  case DAS_OK:
	  case -1:
		break;
	}
  }
  return(rv);
}

