/* ixdrive.c defines the indxr_drive call to the indexer.
 * Calls send_CC() to send message to indexer. Returns:
 *  -1 if nl_response != NLRSP_DIE and CC not present
 *  DAS_OK on success
 *  DAS_UNKN if indexer isn't present
 * $Log$
*/
#include "nortlib.h"
#include "indexer.h"
static char rcsid[] = "$Id$";

int indxr_drive(byte_t drive, byte_t dir, step_t steps) {
  idxr_msg im;
  int rv;
  
  im.msgcode = INDEXER_MSG;
  im.dir_scan = dir & IX_DIR;
  im.drive = drive;
  im.steps = steps;
  rv = send_CC(&im, sizeof(im), 0);
  if (rv == 0) rv = im.msgcode;
  return(rv);
}

