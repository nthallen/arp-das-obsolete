/* server.c */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <windows/Qwindows.h>
#include <string.h>
#include "nortlib.h"
#include "bomem.h"

#define UNKNOWN 1
/* Server command processes messages which begin with "bo"
   (for Bomem!). The subsequent text supported is:
     R<cmd>  Create a proxy for the specified command string
	 p<cmd>  Message is a proxy, no reply required.
	 i       Initialize (if not already initialized)
	 A       Acquire a sample using current settings
	 I       Acquire an interferogram
	 S       Acquire a spectrum
	 N<n>    Set the number of scans
	 Q       Quit
*/
void server_command(pid_t from, char *cmd) {
  int need_reply = 1, all_done = 0;
  char *p;
  struct {
	unsigned short status;
	pid_t proxy;
  } rep;

  p = cmd;
  /* Tell("server", "Got a command: %s", p); */
  if (cmd[0] != 'b' || cmd[1] != 'o') {
	rep.status = UNKNOWN;
  } else {
	cmd += 2;
	if (*cmd == 'R') {
	  *cmd = 'p';
	  rep.proxy = nl_make_proxy(p, strlen(p)+1);
	  rep.status = 0;
	} else {
	  if (*cmd == 'p') {
		need_reply = 0;
		cmd++;
	  }
	  switch (*cmd++) {
		case 'i':		/* Initialize (if not already initialized) */
		  Initialize_DSP();
		  rep.status = 0;
		  rep.proxy = -1;
		  break;
		case 'A':		/* Acquire a sample using current settings */
		  acquire_data();
		  rep.status = 0;
		  rep.proxy = -1;
		  break;
		case 'Q':
		  all_done = 1;
		  rep.status = 0;
		  rep.proxy = -1;
		  break;
		case 'I':		/* Acquire an interferogram */
		case 'S':		/* Acquire a spectrum */
		case 'N':		/*<n> Set the number of scans */
		default:
		  nl_error(2, "Unsupported command string: \"%s\"", p);
		  rep.status = UNKNOWN;
		  break;
	  }
	}
  }
  if (need_reply)
	Reply(from, &rep, sizeof(rep));
  if (all_done) exit(0);
}

/* server_loop() is called when windows is not present */
void server_loop(void) {
  char buf[BOMEM_MSG_MAX+1];
  pid_t from;

  for (;;) {
	from = Receive(0, buf, BOMEM_MSG_MAX+1);
	if (from != -1) {
	  if (buf[0] == 'b' && buf[1] == 'o')
		server_command(from, buf);
	  else reply_byte(from, UNKNOWN);
	}
  }
}
