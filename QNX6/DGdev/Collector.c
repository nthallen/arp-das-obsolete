#include "Collection.h"

collection::collection : data_queue(4,1) {
  regulated = true;
  regulation_optional = false;
}

void collection::init() {
  // Make sure tm_info is defined
  data_queue::init( 1 );
  // Now the dispatch object has been created, so we can add collection devices
}

void collection::service_timer() {
  Collect_Row();
  transmit_data(0);
}

/**
 * Wrapper to link to C row collection function.
 */
void collection::Collect_Row() {
}
