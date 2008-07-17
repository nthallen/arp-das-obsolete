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

void collector::event(enum dg_event evt) {
  if ( evt == dg_event_start ) {
	rowlets = 0;
    next_minor_frame = majf_row = 0;
	minf_row = 0;
    ts_checks =  TSCHK_RTIME | TSCHK_REQUIRED;
  }
}

void collector::commit_tstamp( mfc_t MFCtr, time_t time ) {
  dbr_info.t_stmp.mfc_num = MFCtr;
  dbr_info.t_stmp.secs = time;
  data_generator::commit_tstamp(MFCtr, time);
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
 * Implemented in colmain.skel
 */