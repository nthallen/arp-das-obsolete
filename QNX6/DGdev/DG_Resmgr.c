#include <errno.h>
#include "DG_Resmgr.h"
#include "nortlib.h"

DG_dispatch::DG_dispatch() {
	dpp = dispatch_create();
  if ( dpp == NULL )
    nl_error( 3, "Failed to allocate dispatch handle." );
  single_ctp = dispatch_context_alloc(dpp);
  if ( single_ctp == NULL )
    nl_error(3, "dispatch_context_alloc failed: errno %d", errno );
  quit_received = 0;
}

void DG_dispatch::ready_to_quit() {
	quit_received = 1;
}

DG_dispatch::~DG_dispatch() {
  dispatch_context_free(single_ctp);
  dispatch_destroy(dpp);
}

void DG_dispatch::Loop() {
	dispatch_context_t *ctp = single_ctp;
  while (1) {
    ctp = dispatch_block(ctp);
    if ( ctp == NULL )
      nl_error( 3, "Block error: %d", errno );
    dispatch_handler(ctp);
    if ( quit_received && ctp->resmgr_context.rcvid == 0
	  && all_closed() )
      break;
  }
}
    