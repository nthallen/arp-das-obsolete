#include "Collector.h"

collector::collector : data_queue(4,1) {
  regulated = true;
  regulation_optional = false;
}

void collector::init() {
  // Make sure tm_info is defined
  data_queue::init( 1 );
  // Now the dispatch object has been created, so we can add collector devices
}

void collector::service_timer() {
  Collect_Row();
  transmit_data(0);
}

/**
 * Wrapper to link to C row collector function.
 */
void collector::Collect_Row() {
}
