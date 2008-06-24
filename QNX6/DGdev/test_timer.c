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
#include "nortlib.h"
#include "tm.h"
#include "DG_cmd.h"
#include "DG_tmr.h"

/* return non-zero if a quit command is received */
int DG_cmd::service( int triggered ) {
  for (;;) {
    int rc;
    char buf[BUFSIZE];

    if ( triggered ) {
      nl_error( 0, "Triggered:" );
      rc = read( cmd_fd, buf, BUFSIZE-1 );
      if ( rc < 0 ) nl_error( 2, "Error %d from read", errno );
      else if (rc == 0) {
	nl_error( 0, "Zero returned from read" );
	return 1;
      } else {
	buf[rc] = '\0';
	if (rc > 0 && buf[rc-1] == '\n') buf[rc-1] = '\0';
	nl_error( 0, "Read %d: %s", rc, buf );
      }
    }
    rc = ionotify( cmd_fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_INPUT, &cmd_ev );
    if ( rc < 0 ) nl_error( 3, "Error %d returned from ionotify()", errno );
    if ( rc == 0 ) return 0;
  }
}

DG_cmd::DG_cmd( int coid, int pulse_code ) {
  char *devname;

  devname = tm_dev_name( "cmd/DG" );
  cmd_fd = open( devname, O_RDONLY );
  if ( cmd_fd < 0 ) nl_error( 3, "Unable to read from %s", devname );
  cmd_ev.sigev_notify = SIGEV_PULSE;
  cmd_ev.sigev_coid = coid;
  cmd_ev.sigev_priority = getprio(0);
  cmd_ev.sigev_code = pulse_code;
  if ( service( 0 ) )
    nl_error( 0, "Quit received during initialization" );
}

DG_cmd::~DG_cmd() {
  close(cmd_fd);
}

DG_tmr::DG_tmr( int coid, int pulse_code ) {
  struct sigevent tmr_ev;
  int rc;

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
  int chid, coid, rcvid;
  my_message_t msg;
  int const timer_pulse_code = _PULSE_CODE_MINAVAIL;
  int const cmd_pulse_code = (timer_pulse_code+1);

  chid = ChannelCreate(0);
  coid = ConnectAttach(ND_LOCAL_NODE, 0, 
		  chid, 
		  _NTO_SIDE_CHANNEL, 0);

  DG_cmd cmd(coid, cmd_pulse_code);
  DG_tmr tmr( coid, timer_pulse_code);
  tmr.settime( 0, 500000000L );
  
  for (;;) {
    rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
    if (rcvid == 0) { /* we got a pulse */
      if (msg.pulse.code == timer_pulse_code) {
        printf("we got a pulse from our timer\n");
      } else if ( msg.pulse.code == cmd_pulse_code ) {
	if (cmd.service(1)) break;
      } else nl_error( 2, "Unrecognized pulse code: %d", msg.pulse.code );
    } /* else other messages ... */
  }
  return 0;
}
