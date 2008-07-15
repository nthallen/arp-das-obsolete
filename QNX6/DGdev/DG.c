/* DG.c */
#include <errno.h>
#include "DG.h"

data_generator::data_generator(int nQrows, int low_water)
    : data_queue(nQrows,low_water) {
  bfr_fd = -1;
  quit = false;
  started = false;
  regulated = false;
  regulation_optional = true;
  autostart = false;
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
  tmr->attach();
  row_period_nsec_default = tmi(nsecsper)*(uint64_t)1000000000L/tmi(nrowsper);
  row_period_nsec_current = row_period_nsec_default;
}

/**
 * Control initialization
 * This is how 
 */
void data_generator::operate() {
  if ( autostart ) tm_start(1);
  dispatch->Loop();
}

/**
 Interperet data generator commands: I need to abstract the "signal handlers" operation.
 It appears that the key notifications are:
  Start
  Stop
  Quit
  Change to unregulated output (fast forward)
 
 TM start: "TMc"
    Set start condition
    If regulated, program timer
    else stop timer
    if ext_stop, signal handlers
TM end/stop "TMe"
    Set stop condition
    stop timer
    if ext_time, signal handlers
Quit: ""
    Set stop and quit conditions
    stop timer
    if ext_stop or ext_time, signal handlers
TM play "TM>"
    if regulation_optional
      set regulated
      set row_rate to default
      if stopped, do start
      else reprogram timer
TM fast forward "TM}"
    if regulation_optional
      stop timer
      set unregulated
      if stopped, do start
      else if ext_time, signal handlers
TM faster/slower "TM+" "TM-"
    if regulation_optional
      if stopped
        do play
      else if regulated
        increase/decrease row_rate
        program timer
TM single step "TMs"

Command Summary:
  "" Quit
  "TMc" TM Start
  "TMe" TM End/Stop/Pause
  "TMs" Single Step
  "TM>" Play
  "TM+" Faster
  "TM-" Slower
  "TM}" Fast Forward
  
  Still need to add the search functions:
 TM Advance to MFCtr
 TM Advance to Time
 */
int data_generator::execute(char *cmd) {
  if (cmd[0] == '\0') {
    lock();
    started = false;
    quit = true;
    unlock();
    tmr->settime(0);
    event(dg_event_quit);
    return 1;
  }
  if ( cmd[0] == 'T' && cmd[1] == 'M' ) {
    switch ( cmd[2] ) {
      case 'c': tm_start(1); break;
      case 'e': tm_stop(); break;
      case 's':
        if (started) tm_stop();
        else single_step();
        break;
      case '>': if (regulation_optional) tm_play(); break; // play
      case '+':
        if (regulation_optional) {
          lock();
          if (!started) tm_play(0);
          else {
            if ( regulated ) {
              row_period_nsec_current = row_period_nsec_current * 2 / 3;
              if ( row_period_nsec_current < tmr->timer_resolution_nsec ) {
                regulated = false;
                tmr->settime(0);
                unlock();
                event(dg_event_fast);
              } else {
                tmr->settime(row_period_nsec_current);
                unlock();
              }
            } else unlock();
          }
        }
        break;
      case '-': // slower
        if (regulation_optional) {
          lock();
          if (!started) tm_play(0);
          else {
            row_period_nsec_current = row_period_nsec_current * 3 / 2;
            tmr->settime(row_period_nsec_current);
            regulated = true;
            unlock();
          }
        }
        break;
      case '}': // fast forward
        if (regulation_optional) {
          lock();
          tmr->settime(0);
          regulated = false;
          if (started) {
            unlock();
            event(dg_event_fast);
          } else tm_start(0);
        }
        break;
      default:
        nl_error(2,"Invalid TM command in data_generator::execute: '%s'", cmd );
        break;
    }
  } else nl_error(2, "Invalid command in data_generator::execute: '%s'", cmd );
  return 0;
}

void data_generator::event(enum dg_event evt) {}

void data_generator::tm_start(int lock_needed) {
  if (lock_needed) lock();
  started = true;
  if ( regulated ) tmr->settime(row_period_nsec_current);
  else tmr->settime( 0 );
  unlock();
  event( dg_event_start );
}

void data_generator::tm_play(int lock_needed) {
  if (lock_needed) lock();
  regulated = true;
  row_period_nsec_current = row_period_nsec_default;
  if ( started ) {
    tmr->settime(row_period_nsec_current);
    unlock();
  } else tm_start(0); // don't need to re-lock(), but will unlock()
}

void data_generator::tm_stop() {
  lock();
  started = false;
  unlock();
  tmr->settime(0);
  event(dg_event_stop);
}
