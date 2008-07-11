#ifndef COLLECTOR_H_INCLUDED
#define COLLECTOR_H_INCLUDED
#include "DG.h"

class collector : public data_generator {
  public:
    collector();
    void init();
    void service_timer();
    void Collect_Row();
}

#endif


