/* indexer.h Defines interface to the indexer driver.
 * Indexer will be tailored to the particular drives, ideally
 * through include file specifications.
 * Only one drive may be scanned at a time.
 * Requesting a second scan will produce a DAS_BUSY response.
 * Requesting a drive on a currently scanning device will terminate the scan.
 * Future Features:
 *   Notify/Reply on end of drive/scan
 *
 * &indexer_cmds
 *	: Drive &drive &direction &steps *
 *	: Scan &drive &direction &steps by %u (Enter Steps per Step) *
 *	;
 * &drive <byte_t>
 *	: Etalon
 *	: Attenuator
 *	: Bellows
 *	: Primary Duct Throttle
 *	: Secondary Duct Throttle
 *	;
 * &direction <byte_t>
 *	: In { $0 = IX_IN; }
 *	: Out { $0 = IX_OUT; }
 *	: To { $0 = IX_TO; }
 *	;
 * &steps <step_t>
 *	: %u (Enter Number of Steps or Step Number) { $0 = $1; }
 *	;
 *
 * $Log$
 */
#include <globmsg.h>

typedef unsigned char byte_t;
typedef unsigned int step_t;
typedef struct {
  byte_t msgcode; /* INDEXER_MSG from globmsg.h */
  byte_t dir_scan; /* scan/drive and direction */
  byte_t drive;    /* which drive */
  step_t steps;     /* number of steps or final step */
  step_t dsteps;    /* steps per scan */
} idxr_msg;
#define IX_IN 0
#define IX_OUT 1
#define IX_TO 2
#define IX_DIR 3
#define IX_SCAN 4

#define IX_SCAN_PROXY 255

typedef struct {
  step_t base_addr;
  step_t hysteresis;
} chandef;
#define IX_WITH_HYST 2
#define IX_WOUT_HYST 0

/* Functions return:
   DAS_UNKN if indexer driver is not resident
   DAS_BUSY if second scan is requested
   nl_response fatal or -1 if CC is not resident
*/
int indxr_drive(byte_t drive, byte_t dir, step_t steps);
int indxr_scan(byte_t drive, byte_t dir, step_t steps, step_t dsteps);
