/* ix_cmd.c formats messages to be sent to the indexer
 * $Log$
*/
#include "nortlib.h"
#include "indexer.h"
static char rcsid[] = "$Id$";

int indxr_cmd(byte_t cmd, byte_t drive, step_t steps, step_t dsteps) {
  idxr_msg imsg;
  
  imsg.msgcode = INDEXER_MSG;
  imsg.c.dir_scan = cmd;
  imsg.c.drive = drive;
  imsg.c.steps = steps;
  imsg.c.dsteps = dsteps;
  if (send_CC(&imsg, sizeof(imsg), 0) == -1) return(-1);
  if (imsg.msgcode != DAS_OK && nl_response)
	nl_error(nl_response, "Error requesting indexer cmd %d", cmd);
  return(imsg.msgcode);
}
