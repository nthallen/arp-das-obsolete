/* Header file for the Bodma Program */

#include "port_types.h"

#define BODMA_STRING "Bo"  /* used with nort's Col_send_init */
#define BODMA "bodma"                   /* my attached name */
#define SEQ36 25


/* An Acqusition Request specifies number of Coadd Sequences and Number
   of Scans to Coadd for each Sequence */
/* Status */
#define BO_CLOSE 0 /* Driver Not Installed */
#define BO_READY 1 /* Driver Ready for Acquisition Requests */
#define BO_INIT 2  /* Driver Initialising */
#define BO_START 3 /* Driver Starting an Acquisition Request */
#define BO_ACQ 4   /* Driver Acquiring Request, Awaiting Data */
#define BO_DATA 5  /* A Coadd Sequence is Available */
#define BO_LOG 6   /* Driver Logging Data */

/* Structure that is passed to Collection */
typedef struct {
  UBYTE2 seq;       /* sequence number of file */
  UBYTE1 status;    /* status */
  BYTE4 scans;      /* Number of scans in coadd */
} bodma_frame;

typedef struct {
  BYTE1 hdr;
  BYTE2 scans;/* 0 : single IR; >0 : double IR, # scans/coadd; <0 : stop */
  UBYTE2 runs; /* Number of Measurements */
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
} bo_file_header;

