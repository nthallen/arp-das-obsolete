#ifndef DG_TMR_H
#define DG_TMR_H

#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include "nortlib.h"

class DG_tmr {
  public:
    DG_tmr(dispatch_t *dpp );
    ~DG_tmr();
    void settime( int per_sec, int per_nsec );
  private:
    int timerid;
};

extern "C" {
	int DG_tmr_pulse_func( message_context_t *ctp, int code,
		unsigned flags, void *handle );
}

#endif

