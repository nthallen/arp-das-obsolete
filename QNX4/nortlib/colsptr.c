/* Col_set_pointer passes a pointer to the DG (collection or extraction)
   for shared memory features. The flags argument is passed to
   qnx_segment_arm() and can be 0 by default, although I envision
   specifying _PMF_DATA_R in most cases.
   The id is the id specified in the TMC input file for the
   corresponding pointer there.
   Col_set_pointer returns zero on success. Possible error
   conditions are failure to locate the DG or an error returned
   by the DG when the pointer is passed. All errors are considered
   fatal in the sense of nl_error(3, ...). That is to say, without
   modifying the nl_error pointer, Col_set_pointer will die on
   any error, making error checking unnecessary. nl_error could
   be intercepted if the error conditions are tolerable.
*/
#include <sys/seginfo.h>
#include <sys/kernel.h>
#include "collect.h"
#include "nortlib.h"
#include "globmsg.h"
char rcsid_colsptr_c[] =
  "$Header$";

int Col_set_pointer(unsigned char id, void *pointer, unsigned flags) {
  struct colmsg c;
  pid_t dgpid;
  int rv;
  
  c.type = COL_SET_POINTER;
  c.id = id;
  c.u.pointer = (void far *)pointer;
  dgpid = find_DG();
  if (dgpid > 0) {
	qnx_segment_arm(dgpid, FP_SEG(c.u.pointer), flags);
	if ((rv = send_DG(&c, sizeof(struct colmsg))) == 0
		&& c.type != DAS_OK) {
	  rv = -1;
	  if (nl_response)
		nl_error(nl_response, "Error from DG setting pointer");
	}
  } else rv = -1;
  return(rv);
}

