/* ixscan.c defines the indxr_scan call to the indexer.
 * Calls send_CC() to send message to indexer. Returns:
 *  -1 if nl_response != NLRSP_DIE and CC not present
 *  DAS_OK on success
 *  DAS_UNKN if indexer isn't present && nl_response != NLRSP_DIE
*/
#include "nortlib.h"
#include "indexer.h"
char rcsid_ixscan_c[] =
  "$Header$";

int indxr_scan(byte_t drive, byte_t dir, step_t steps, step_t dsteps) {
  return(indxr_cmd((dir & IX_DIR) | IX_SCAN, drive, steps, dsteps));
}
