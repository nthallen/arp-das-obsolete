#include "Collector.h"

collector::collector() : data_generator(4,1) {
  regulated = true;
  regulation_optional = false;
}

void collector::init() {
  data_generator::init( 1 );
  tminitfunc();
}

void collector::service_timer() {
  Collect_Row();
  transmit_data(0);
}

/**
 * Wrapper to link to C row collector function.
 */
//void collector::Collect_Row() { }
