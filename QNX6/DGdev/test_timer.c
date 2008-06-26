/* Timer test
   Create a channel
   Create a connection to the channel
   Define an event as Pulse to connection
   Create a timer using event
   Set the timer
   MsgReceive() waiting for the pulse and print something
*/
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/iomsg.h>
#include "tm.h"
#include "DG_Resmgr.h"
#include "DG_cmd.h"
#include "DG_tmr.h"

int DG_tmr_pulse_func( message_context_t *ctp, int code,
		unsigned flags, void *handle ) {
  nl_error( 0, "Received timer pulse" );
  return 0;
}

DG_tmr::DG_tmr( dispatch_t *dpp ) {
  struct sigevent tmr_ev;
  int rc;

  int pulse_code =
    pulse_attach( dpp, MSG_FLAG_ALLOC_PULSE, 0, DG_tmr_pulse_func, NULL );
  if ( pulse_code < 0 )
    nl_error(3, "Error %d from pulse_attach", errno );
  int coid = message_connect( dpp, MSG_FLAG_SIDE_CHANNEL );
  if ( coid == -1 )
    nl_error(3, "Error %d from message_connect", errno );
  tmr_ev.sigev_notify = SIGEV_PULSE;
  tmr_ev.sigev_coid = coid;
  tmr_ev.sigev_priority = getprio(0);
  tmr_ev.sigev_code = pulse_code;
  rc = timer_create( CLOCK_REALTIME, &tmr_ev, &timerid );
  if ( rc < 0 ) nl_error( 3, "Error creating timer" );
}

DG_tmr::~DG_tmr() {
  // timer_delete?();
  nl_error( 0, "Destructing DG_tmr object" );
}

void DG_tmr::settime( int per_sec, int per_nsec ) {
  struct itimerspec itime;

  itime.it_value.tv_sec = itime.it_interval.tv_sec = per_sec;
  itime.it_value.tv_nsec = itime.it_interval.tv_nsec = per_nsec;
  timer_settime(timerid, 0, &itime, NULL);
}

typedef union {
  struct _pulse pulse;
  /* your other message structures would go here too */
} my_message_t;

int main(int argc, char **argv) {
	DG_dispatch dispatch;


  DG_cmd cmd( &dispatch );
  DG_tmr tmr( dispatch.dpp );
  tmr.settime( 0, 500000000L );

  dispatch.Loop();
  return 0;
}
