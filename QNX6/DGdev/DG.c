/* DG.c */
#include "DG.h"

data_generator::data_generator(int nQrows, int low_water)
    : data_queue(nQrows,low_water) {
  bfr_fd = -1;
}

/**
 * Assumes tm_info is defined
 */
void data_generator::init(int collection) {
  data_queue::init();
  bfr_fd = open(tm_dev_name("TM/DG"), collection ? O_WRONLY|O_NONBLOCK : O_WRONLY );
  if (bfr_fd < 0) nl_error(3, "Unable to open TM/DG: %d", errno );

	dispatch = new DG_dispatch();
  cmd = new DG_cmd(this);
  cmd->attach();
  tmr = new DG_tmr(this);
  tmr->attach( dispatch );
}

/**
 * Control initialization
 * This is how 
 */
void data_generator::operate() {
  if ( autostart ) tm_start();
  dispatch.Loop();
}
