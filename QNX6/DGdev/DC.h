#ifndef DC_H_INCLUDED
#define DC_H_INCLUDED

#include "DataQueue.h"

class data_client : public data_queue {
  public:
    data_client(int nQrows, int low_water);
    void init();
    void operate(); // event loop
  protected:
    void process_data();
    int bfr_fd;
  private:
};

#endif
