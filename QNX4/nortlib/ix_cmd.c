/* ix_cmd.c formats messages to be sent to the indexer
 * $Log$
 * Revision 1.1  1993/02/18  02:29:07  nort
 * Initial revision
 *
*/
#include "nortlib.h"
#include "indexer.h"
#ifdef __WATCOMC__
  #pragma off (unreferenced)
	static char rcsid[] =
	  "$Id$";
  #pragma on (unreferenced)
#endif

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
