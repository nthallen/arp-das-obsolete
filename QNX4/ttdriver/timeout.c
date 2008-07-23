#include <signal.h>
#include <time.h>
#include "nortlib.h"

static timer_t tid;
static struct itimerspec value;
static struct itimerspec unvalue;
int timed_out = 0;

void MyHandler( int signum ) {
  signum = signum;
  timed_out = 1;
}

void init_timer( void ) {
  tid = timer_create( CLOCK_REALTIME, NULL );
  if ( tid == -1 )
	nl_error( 3, "Unable to allocate timer" );
  signal( SIGALRM, MyHandler );
  value.it_value.tv_sec = 0;
  value.it_value.tv_nsec = 250000000;
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_nsec = 0;
  unvalue.it_value.tv_sec = 0;
  unvalue.it_value.tv_nsec = 0;
  unvalue.it_interval.tv_sec = 0;
  unvalue.it_interval.tv_nsec = 0;
}

void start_timer( void ) {
  timer_settime( tid, 0, &value, NULL );
  timed_out = 0;
}

void stop_timer( void ) {
  timer_settime( tid, 0, &unvalue, NULL );
}

