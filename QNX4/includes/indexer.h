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
 *	: Scan &drive &direction &steps by %d (Enter Steps per Step) *
 *	: Stop &drive *
 *	: Drive &drive Online *
 *	: Drive &drive Offline *
 *	: Set &chop_drive Online Position %d (Enter Online Position) *
 *	: Set &chop_drive Online Delta
 *	   %d (Enter positive number of steps between dithered online positions) *
 *	: Set &chop_drive Offline Delta
 *	   %d (Enter signed number of steps from Online to Offline position) *
 *	: Move &chop_drive Online Position Out *
 *	: Move &chop_drive Online Position In *
 *	;
 * &drive <byte_t>
 *	: &chop_drive
 *	: Bellows
 *	: Primary Duct Throttle
 *	: Secondary Duct Throttle
 *	;
 * &chop_drive <byte_t>
 *	: Etalon
 *	: Attenuator
 *	;
 * &direction <byte_t>
 *	: In { $0 = IX_IN; }
 *	: Out { $0 = IX_OUT; }
 *	: To { $0 = IX_TO; }
 *	;
 * &steps <step_t>
 *	: %d (Enter Number of Steps or Step Number) { $0 = $1; }
 *	;
 *
 * $Log$
 * Revision 1.5  1993/01/09  15:50:52  nort
 * *** empty log message ***
 *
 * Revision 1.4  1992/09/24  20:23:10  nort
 * With Command Queueing
 *
 * Revision 1.3  1992/09/24  13:47:36  nort
 * Working, but prior to command queueing
 *
 * Revision 1.2  1992/09/02  20:15:16  nort
 * Closer to a release edition.
 *
 * Revision 1.1  1992/09/02  16:41:20  nort
 * Initial revision
 *
 */
#ifndef INDEXER_H_INCLUDED
#define INDEXER_H_INCLUDED
#include <globmsg.h>

typedef unsigned char byte_t;
typedef unsigned int step_t;
typedef struct {
  byte_t dir_scan; /* scan/drive and direction */
  byte_t drive;    /* which drive (and hysteresis info) */
  step_t steps;     /* number of steps or final step */
  step_t dsteps;    /* steps per scan */
} ixcmd;

typedef struct {
  byte_t msgcode; /* INDEXER_MSG from globmsg.h */
  ixcmd c;
} idxr_msg;
#define IX_IN 0
#define IX_OUT 1
#define IX_TO 2
#define IX_DIR 3
#define IX_SCAN 4
#define IX_STOP 8
#define IX_ONLINE 9
#define IX_OFFLINE 10
#define IX_MOVE_ONLINE_OUT 11
#define IX_MOVE_ONLINE_IN 12
#define IX_SET_ONLINE 13
#define IX_SET_ON_DELTA 14
#define IX_SET_OFF_DELTA 15
#define IX_SET_NO_LOOPS 16

/* This is set in the drive byte */
#define IX_BELLOWS 0
#define IX_ETALON 1
#define IX_ATTENUATOR 2
#define IX_PRIMARY_TV 3
#define IX_SECONDARY_TV 4
#define IX_USE_HYSTERESIS 0x80
#define N_CHANNELS 5

#define INDEXER_FLAG_ID 1
#define INDEXER_PROXY_ID 2
#define ETN_ON_PROXY_ID 3
#define ETN_OFF_PROXY_ID 4

/* Status Bits in the flag word */
#define BLW_SCAN_BIT 1
#define ETN_SCAN_BIT 2
#define ETN_CHOP_BIT 4
#define ETN_SUPP_BIT 8
#define ATN_SCAN_BIT 0x10
#define ATN_CHOP_BIT 0x20
#define PTV_SCAN_BIT 0x40
#define STV_SCAN_BIT 0x100
#define NO_LOOPS_BIT 0x80
/* Functions return:
   DAS_UNKN if indexer driver is not resident
   DAS_BUSY if second scan is requested
   nl_response fatal or -1 if CC is not resident
*/
int indxr_cmd(byte_t cmd, byte_t drive, step_t steps, step_t dsteps);
int indxr_drive(byte_t drive, byte_t dir, step_t steps);
int indxr_scan(byte_t drive, byte_t dir, step_t steps, step_t dsteps);
int indxr_stop(byte_t drive);
int indxr_online(byte_t drive);
int indxr_offline(byte_t drive);
int indxr_move_out(byte_t drive);
int indxr_move_in(byte_t drive);
int indxr_set_online(byte_t drive, step_t steps);
int indxr_online_delta(byte_t drive, step_t steps);
int indxr_offline_delta(byte_t drive, int steps);
#endif
