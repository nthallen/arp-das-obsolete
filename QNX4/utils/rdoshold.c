#include <stdio.h>
#include <fcntl.h>
#include <sys/console.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/proxy.h>
#include <sys/kernel.h>
#include <signal.h>
#include <process.h>
#include <sys/qnx_glob.h>
#include "nortlib.h"

#ifdef __USAGE
%C	<program>
	<program> is an rdos program.
#endif

static int child_terminated = 0;

static void handler(int sig) {
  child_terminated = sig;
}

int main(int argc, char **argv) {
  struct _console_ctrl *cc;
  struct _console_info info;
  struct sigaction act;
  unsigned events, state;
  pid_t proxy, pid, child;

  /* Check argument */
  if (argc < 2) nl_error(3, "Must specify a rdos program");

  /* Open the console and arm it. */  
  cc = console_open(STDOUT_FILENO, O_RDWR);
  if (cc == NULL)
	nl_error(3, "Unable to open stdout as console: %s", strerror(errno));
  proxy = qnx_proxy_attach(0, 0, 0, -1);
  events = _CON_EVENT_ACTIVE | _CON_EVENT_INACTIVE;
  console_state(cc, 0, 0, events);
  console_arm(cc, 0, proxy, events);

  /* check to see if it is active currently */
  console_info(cc, -1, &info);
  if (info.console != cc->console)
	qnx_spawn_options.flags = _SPAWN_HOLD;

  /* Define the signal */
  act.sa_handler = handler;
  act.sa_mask = 0;
  act.sa_flags = SA_NOCLDSTOP;
  if (sigaction(SIGCHLD, &act, NULL) == -1)
	nl_error(3, "Unable to define handler: %s", strerror(errno));

  /* Start up child process */
  { char **nargv;
	int i;
	
    nargv = malloc((argc+1)*sizeof(char *));
	if (nargv == 0) nl_error(4, "No memory");
	nargv[0] = "rdos";
	for (i = 1; i < argc; i++) nargv[i] = argv[i];
	nargv[argc] = NULL;
	child = spawnvp(P_NOWAIT, nargv[0], nargv);
  }
  if (child == -1) nl_error(3, "Unable to spawn program");

  for (;;) {
	pid = Receive(0, NULL, 0);
	if (pid == proxy) {
	  state = console_state(cc, 0, 0, events);
	  if (state & _CON_EVENT_ACTIVE) kill(child, SIGCONT);
	  if (state & _CON_EVENT_INACTIVE) kill(child, SIGSTOP);
	  console_arm(cc, 0, proxy, events);
	} else if (pid == -1) {
	  if (errno == EINTR && child_terminated) break;
	  else nl_error(3, "Error receiving: %s", strerror(errno));
	} else nl_error(3, "Received from someone else");
  }
  console_close(cc);
  printf("\f\n");
  return(0);
}
