/* send_tmr.c consolidates function of sending messages to the timerbd.
 */
#include <errno.h>
#include <sys/kernel.h>
#include "nortlib.h"
#include "timerbd.h"
char rcsid_send_tmr_c[] =
  "$Header$";

/* send_Tmr does not report DAS_* errors, since they are better handled
   by the other library functions which call this one.
 */
int send_Tmr(struct tmrbdmsg *rqst) {
  pid_t TmrPID;
  int rv;
  
  if ((TmrPID = find_Tmr()) == -1) return(-1);
  while ((rv = Send(TmrPID, rqst, rqst,	sizeof(struct tmrbdmsg),
					sizeof(struct tmrbdmsg))) == -1
		 && errno == EINTR);
  if (rv == -1) {
	if (nl_response)
	  nl_error(nl_response, "Error sending to timerbd");
  } else rv = rqst->op_rtn;
  return(rv);
}
