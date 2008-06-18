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

#define TIMER_PULSE_CODE _PULSE_CODE_MINAVAIL
#define CMD_PULSE_CODE (TIMER_PULSE_CODE+1)

typedef union {
  struct _pulse pulse;
  /* your other message structures would go here too */
} my_message_t;

#define TM_BUFSIZE 80

/* return non-zero if a quit command is received */
int service_tm( int tm_fd, struct sigevent *ev, int triggered ) {
  for (;;) {
    int rc;
    char buf[TM_BUFSIZE];

    if ( triggered ) {
      nl_error( 0, "Triggered:" );
      rc = read( tm_fd, buf, TM_BUFSIZE-1 );
      if ( rc < 0 ) nl_error( 2, "Error %d from read", errno );
      else if (rc == 0) {
	nl_error( 0, "Zero returned from read" );
	return 1;
      } else {
	buf[rc] = '\0';
	nl_error( 0, "Read %d: %s", rc, buf );
      }
    }
    rc = ionotify( tm_fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_INPUT, ev );
    if ( rc < 0 ) nl_error( 3, "Error %d returned from ionotify()", errno );
    if ( rc == 0 ) return 0;
  }
}

int main(int argc, char **argv) {
  int rc;
  struct sigevent ev;
  timer_t timerid;
  int chid, rcvid;
  struct itimerspec itime;
  my_message_t msg;
  char *devname;
  int tm_fd;

  chid = ChannelCreate(0);

  ev.sigev_notify = SIGEV_PULSE;
  ev.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, 
                                  chid, 
                                  _NTO_SIDE_CHANNEL, 0);
  ev.sigev_priority = getprio(0);
  ev.sigev_code = TIMER_PULSE_CODE;
  rc = timer_create( CLOCK_REALTIME, &ev, &timerid );

  itime.it_value.tv_sec = 1;
  /* 500 million nsecs = .5 secs */
  itime.it_value.tv_nsec = 500000000; 
  itime.it_interval.tv_sec = 1;
  /* 500 million nsecs = .5 secs */
  itime.it_interval.tv_nsec = 500000000; 
  timer_settime(timerid, 0, &itime, NULL);

  /* Now open command server for reading */
  devname = tm_dev_name( "cmd/tm" );
  tm_fd = open( devname, O_RDONLY );
  if ( tm_fd < 0 ) nl_error( 3, "Unable to read from %s", devname );
  ev.sigev_code = CMD_PULSE_CODE;
  if ( service_tm( tm_fd, &ev, 0 ) )
    nl_error( 0, "Quit received during initialization" );
  
  for (;;) {
    rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
    if (rcvid == 0) { /* we got a pulse */
      if (msg.pulse.code == TIMER_PULSE_CODE) {
        printf("we got a pulse from our timer\n");
      } else if ( msg.pulse.code == CMD_PULSE_CODE ) {
	if (service_tm(tm_fd, &ev, 1)) break;
      } else nl_error( 2, "Unrecognized pulse code: %d", msg.pulse.code );
    } /* else other messages ... */
  }
  return 0;
}
