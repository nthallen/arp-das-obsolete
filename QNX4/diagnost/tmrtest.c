#include <stdlib.h>
#include <sys/kernel.h>
#include "nortlib.h"
#include "timerbd.h"
#include "msg.h"

#define MAX_TIMERS 10

char *opt_string=OPT_MSG_INIT;

void main(int argc, char **argv) {
  int n_timers, max_timer;
  int timers[MAX_TIMERS];
  unsigned char msgv;
  
  msg_init_options("TmrTest", argc, argv);
  set_response(1);
  for (n_timers = 0; ; n_timers++) {
	timers[n_timers] = Tmr_proxy(TMR_ONCE, 0, n_timers);
	if (timers[n_timers] < 0) break;
  }
  msg(MSG, "Initialized %d timers", n_timers);
  max_timer = n_timers;
  while (n_timers > 0) {
	Receive(0, &msgv, 1);
	if (msgv >= max_timer)
	  msg(MSG_WARN, "Received unexpected message %d", msgv);
	else if (timers[msgv] == -1)
	  msg(MSG_WARN, "Received message %d more than once", msgv);
	else {
	  msg(MSG, "Observed timer %d", timers[msgv]);
	  Tmr_reset(timers[msgv]);
	  timers[msgv] = -1;
	  n_timers--;
	}
  }
  exit(0);
}
