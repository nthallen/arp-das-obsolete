/* ixdrive.c defines the indxr_drive call to the indexer.
 * Calls send_CC() to send message to indexer. Returns:
 *  -1 if nl_response != NLRSP_DIE and CC not present
 *  DAS_OK on success
 *  DAS_UNKN if indexer isn't present && nl_response != NLRSP_DIE
 * $Log$
 * Revision 1.3  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.2  1993/02/18  02:27:55  nort
 * Simplified via use of indxr_cmd()
 *
 * Revision 1.1  1992/09/02  20:16:15  nort
 * Initial revision
 *
*/
#include "nortlib.h"
#include "indexer.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

int indxr_drive(byte_t drive, byte_t dir, step_t steps) {
  return(indxr_cmd(dir & IX_DIR, drive, steps, 0));
}

