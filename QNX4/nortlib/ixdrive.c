/* ixdrive.c defines the indxr_drive call to the indexer.
 * Calls send_CC() to send message to indexer. Returns:
 *  -1 if nl_response != NLRSP_DIE and CC not present
 *  DAS_OK on success
 *  DAS_UNKN if indexer isn't present && nl_response != NLRSP_DIE
 * $Log$
 * Revision 1.1  1992/09/02  20:16:15  nort
 * Initial revision
 *
*/
#include "nortlib.h"
#include "indexer.h"
static char rcsid[] = "$Id$";

int indxr_drive(byte_t drive, byte_t dir, step_t steps) {
  return(indxr_cmd(dir & IX_DIR, drive, steps, 0));
}

