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
#include <sys/neutrino.h>
#include <sys/netmgr.h>

#define MY_PULSE_CODE _PULSE_CODE_MINAVAIL

typedef union {
  struct _pulse pulse;
  /* your other message structures would go here too */
} my_message_t;

int main(int argc, char **argv) {
  int rc;
  struct sigevent ev;
  timer_t timerid;
  int chid, rcvid;
  struct itimerspec itime;
  my_message_t msg;

  chid = ChannelCreate(0);

  ev.sigev_notify = SIGEV_PULSE;
  ev.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, 
                                  chid, 
                                  _NTO_SIDE_CHANNEL, 0);
  ev.sigev_priority = getprio(0);
  ev.sigev_code = MY_PULSE_CODE;
  rc = timer_create( CLOCK_REALTIME, &ev, &timerid );

  itime.it_value.tv_sec = 1;
  /* 500 million nsecs = .5 secs */
  itime.it_value.tv_nsec = 500000000; 
  itime.it_interval.tv_sec = 1;
  /* 500 million nsecs = .5 secs */
  itime.it_interval.tv_nsec = 500000000; 
  timer_settime(timerid, 0, &itime, NULL);
  for (;;) {
    rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
    if (rcvid == 0) { /* we got a pulse */
      if (msg.pulse.code == MY_PULSE_CODE) {
        printf("we got a pulse from our timer\n");
      } /* else other pulses ... */
    } /* else other messages ... */
  }

}
