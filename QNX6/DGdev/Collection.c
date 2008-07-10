#include "Collection.h"

collection::collection : data_queue(1,4,1) {
  regulated = true;
  regulation_optional = false;
}

void collection::service_timer() {
  Collect_Row();
  transmit_data(0);
}

void collection::Collect_Row() {
}
