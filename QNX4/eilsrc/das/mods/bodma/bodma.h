/* Header file for the Bodma Program */

#include "port_types.h"

#define BODMA_STRING "bodma_data"  /* used with nort's Col_send_init */
#define BODMA "bodma"                   /* my attached name */
#define SEQ36 25

/* Structure that is passed to Collection */
typedef struct {
  UBYTE2 seq;       /* sequence number of file */
  UBYTE1 status;    /* status */
  BYTE4 scans;      /* Number of scans in coadd */
} bodma_frame;

typedef struct {
  BYTE1 hdr;
  BYTE4 scans;/* 0 : single IR; >0 : double IR, # scans/coadd; <0 : stop */
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

