#ifndef SUBBUSD_H_INCLUDED
#define SUBBUSD_H_INCLUDED
#include <sys/iomsg.h>

#define SUBBUSD_MGRID_OFFSET 1
#define SUBBUSD_MAX_REQUEST 25
#define SUBBUSD_MGRID (_IOMGR_PRIVATE_BASE + SUBBUSD_MGRID_OFFSET)

typedef struct {
  struct _io_msg hdr;
  char request[SUBBUSD_MAX_REQUEST];
} subbusd_msg_t;

#endif
