#ifndef COLLECTION_H_INCLUDED
#define COLLECTION_H_INCLUDED
#include "DataQueue.h"

class collection : public data_generator {
  public:
    collection();
    void init();
    void service_timer();
    void Collect_Row();
}

#endif


