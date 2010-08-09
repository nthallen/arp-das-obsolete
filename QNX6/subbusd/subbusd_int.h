#ifndef SUBBUSD_INT_H_INCLUDED
#define SUBBUSD_INT_H_INCLUDED
#include <sys/dispatch.h>
#include "subbusd.h"

extern void incoming_sbreq( int rcvid, char *req );
extern void init_subbus(dispatch_t *dpp );
extern void shutdown_subbus(void);

#endif
