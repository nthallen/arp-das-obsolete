/* ixscan.c defines the indxr_scan call to the indexer.
 * Calls send_CC() to send message to indexer. Returns:
 *  -1 if nl_response != NLRSP_DIE and CC not present
 *  DAS_OK on success
 *  DAS_UNKN if indexer isn't present && nl_response != NLRSP_DIE
 * $Log$
 * Revision 1.2  1993/02/18  02:28:36  nort
 * Simplified via use of indxr_cmd()
 *
 * Revision 1.1  1992/09/02  20:16:15  nort
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

int indxr_scan(byte_t drive, byte_t dir, step_t steps, step_t dsteps) {
  return(indxr_cmd((dir & IX_DIR) | IX_SCAN, drive, steps, dsteps));
}
