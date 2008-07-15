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
 * Collect_Row() is reponsible for:
 * -determining whether a new timestamp is required
 * -filling in/defining the minor fram counter and synch
 * -populating the row of data
 * New timestamp may be required because:
 * -we just started
 * -the minor frame counter is rolling over
 * -we are greater than TS_MFC_LIMIT minor frames from the old timestamp
 * -we have drifted from realtime somehow
 */
void collector::Collect_Row() {
}
