#ifndef DG_H_INCLUDED
#define DG_H_INCLUDED

#include "DataQueue.h"

class data_generator : public data_queue {
  public:
    data_generator(int nQrows, int low_water);
    void init( int collection );
    void operate(); // event loop
  protected:
    virtual void service_timer();
    int transmit_data( int single_row );
    int bfr_fd;
    DG_dispatch *dispatch;
    DG_cmd *cmd;
    DG_tmr *tmr;
}

#endif
