/* Header file for the Bodma Program */

#include "port_types.h"

#define BODMA_SLOW_STRING "Bo_Slow" /* used with nort's Col_send_init */
#define BODMA_FAST_STRING "Bo_Fast" /* used with nort's Col_send_init */
#define BODMA "bodma"      /* my attached name */
#define SEQ36 25
#define BO_FILE_VERSION 0  /* orig */

/* An Acqusition Request specifies number of Coadd Sequences and Number
   of Scans to Coadd for each Sequence */

/* Status Bits */
#define BO_OPEN      0x01 /* Driver Installed */
#define BO_ACQ       0x02 /* Driver Servicing an Acquisition Run Request */
#define BO_DATA      0x04 /* Driver Acquired a Sequence Measurement */
#define BO_LOG       0x08 /* Driver Logging a Sequence Measurement */
#define BO_PEN       0x10 /* Penultimate Scan in Coadd Occured */
#define BO_ZPD       0x20 /* ZPD Computation On */

/* Fast Bomem Status that is passed to Collection */
typedef struct {
  UBYTE1 status;
} bodma_fast_frame;

/* Slow Structure that is passed to Collection */
typedef struct {
  UBYTE2 seq;       /* sequence number of file */
  BYTE4 scans;      /* Number of scans in coadd */
  BYTE4 A_l_zpd_pos;
  BYTE4 A_l_zpd_neg;
  float A_zpd_pos;
  float A_zpd_neg;
  BYTE4 B_l_zpd_pos;
  BYTE4 B_l_zpd_neg;
  float B_zpd_pos;
  float B_zpd_neg;
} bodma_slow_frame;

typedef struct {
  BYTE1 hdr;
  BYTE2 scans;/* 0 : single IR; >0 : double IR, # scans/coadd; <0 : stop */
  UBYTE2 runs; /* Number of Measurements */
  BYTE1 zpd; /* ~0 : Turn On ZPD computing; 0: Turn Off ZPD computing */
  BYTE1 david_code;
  BYTE1 david_pad[8];
} Seq36_Req;

typedef struct {
BYTE1 version;
UBYTE2 seq;
UBYTE4 time;
BYTE4 scans;
UBYTE4 npts;
BYTE1 david_code;
BYTE1 david_pad[8];
/* float zpd_pos;
float zpd_neg;
*/
} bo_file_header;

