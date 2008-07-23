#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/kernel.h>
#include <sys/timers.h>
#include <sys/proxy.h>
#include "nortlib.h"
#include "timerbd.h"
#include "msg.h"

extern int (*nl_error)(int level, char *s, ...) = msg;

struct sys_tmr {
  timer_t timerid;
  pid_t proxy;
};

void sys_tmr_proxy(struct sys_tmr *t, unsigned short per, unsigned char msg) {
  struct sigevent evp;
  struct itimerspec value;
  
  t->proxy = nl_make_proxy(&msg, 1);
  evp.sigev_signo = -t->proxy;
  t->timerid = timer_create(CLOCK_REALTIME, &evp);
  if (t->timerid == -1)
	nl_error(3, "Error making timer");
  value.it_value.tv_sec = per/TMR_0_FREQ;
  value.it_value.tv_nsec = (per%TMR_0_FREQ)*61035L;
  value.it_interval.tv_sec = value.it_value.tv_sec;
  value.it_interval.tv_nsec = value.it_value.tv_nsec;
  timer_settime(t->timerid, 0, &value, NULL);
}

void sys_tmr_delete(struct sys_tmr *t) {
  timer_delete(t->timerid);
  qnx_proxy_detach(t->proxy);
}

#define MAX_TIMERS 10

char *opt_string=OPT_MSG_INIT;

void main(int argc, char **argv) {
  int n_timers, max_timer, n_sys_times;
  int timers[MAX_TIMERS];
  unsigned char msgv;
  struct sys_tmr t;
  
  msg_init_options("TmrTest", argc, argv);
  set_response(0);

  /* Program timers for 1 second time-out */
  for (n_timers = 0; ; n_timers++) {
	timers[n_timers] = Tmr_proxy(TMR_ONCE, TMR_0_FREQ, n_timers);
	if (timers[n_timers] < 0) break;
  }
  set_response(3);
  if (n_timers == 0) msg(3, "No timers available!");

  sys_tmr_proxy(&t, TMR_0_FREQ/2, n_timers);

  msg(MSG, "Initialized %d timers", n_timers);
  max_timer = n_timers;
  n_sys_times = 0;
  while (n_timers > 0 && n_sys_times < 3) {
	Receive(0, &msgv, 1);
	if (msgv == max_timer) n_sys_times++;
	else if (msgv > max_timer)
	  msg(MSG_WARN, "Received unexpected message %d", msgv);
	else if (timers[msgv] == -1)
	  msg(MSG_WARN, "Received message %d more than once", msgv);
	else {
	  if (n_sys_times < 1)
		msg(MSG_FAIL, "Observed timer %d early", timers[msgv]);
	  else msg(MSG, "Observed timer %d", timers[msgv]);
	  Tmr_reset(timers[msgv]);
	  timers[msgv] = -1;
	  n_timers--;
	}
  }
  sys_tmr_delete(&t);
  if (n_timers > 0) {
	for (n_timers = 0; n_timers < max_timer; n_timers++) {
	  if (timers[n_timers] != -1) {
		msg(MSG_FAIL, "Timer %d never observed", timers[n_timers]);
		/* I will release timer 1 because it is used without
		   the interrupt. Any other timer, I will not release
		   so it won't be used. */
		if (timers[n_timers] == 1) Tmr_reset(timers[n_timers]);
	  }
	}
	exit(1);
  }
  exit(0);
}
