#ifndef SERUSB_H_INCLUDED
#define SERUSB_H_INCLUDED
#include "subbusd_int.h"

typedef struct {
  int type;
  int status;
  int rcvid;
  char request[SUBBUSD_MAX_REQUEST];
} sbd_request_t;

#define SBDR_TYPE_INTERNAL 0
#define SBDR_TYPE_CLIENT 1
#define SBDR_TYPE_MAX 1
#define SBDR_STATUS_QUEUED 0
#define SBDR_STATUS_SENT 1

/* SUBBUSD_MAX_REQUESTS is the size of the request queue,
   so it determines the number of simultaneous requests 
   we can handle. Current usage suggests we will have
   a small number of programs accessing the subbus
   (col,srvr,idxr,dccc,ana104,card,digital) so 20 is
   not an unreasonable upper bound */
#define SUBBUSD_MAX_REQUESTS 20

#endif
