/* congetch.c contains nlcon_getch() */
#include <assert.h>
#include <unistd.h>
#include <sys/dev.h>
#include <sys/kernel.h>
#include <sys/proxy.h>
#include "nl_cons.h"
char rcsid_congetch_c[] =
	"$Header$";

static void arm_cons(void) {
  unsigned int i, n_armed;
  nl_con_def *con;
  
  for (n_armed = 0, i = 0; i < MAXCONS; i++) {
    con = &nl_cons[i];
	if (con->con_ctrl != NULL) {
	  con->old_mode = dev_mode(con->fd, 0, _DEV_ECHO | _DEV_EDIT | _DEV_ISIG);
	  { struct _dev_info_entry info;

		con->proxy = qnx_proxy_attach(0, NULL, 0, -1);
		if (con->proxy == -1)
		  nl_error(3, "Error attaching proxy");
		else if (dev_info(con->fd, &info))
		  nl_error(3, "Error getting dev info");
		else {
		  con->nid = info.nid;
		  con->rproxy = qnx_proxy_rem_attach(con->nid, con->proxy);
		  if (con->rproxy == -1)
			nl_error(3, "Error getting remote proxy");
		  else if (dev_arm(con->fd, con->rproxy, _DEV_EVENT_INPUT) == -1)
			nl_error(3, "Error arming console");
		  else n_armed++;
		}
	  }
	}
  }
  if (n_armed == 0) nl_error(3, "No consoles available for input");
}

int nlcon_getch(void) {
  pid_t who;
  int i;
  static unsigned char ibuf[IBUFSZ], *bptr;
  static unsigned int nchars = 0;
  static int consoles_armed = 0;
  
  if (!consoles_armed) {
	arm_cons();
	consoles_armed = 1;
  }

  while (nchars == 0) {
	who = Receive(0, NULL, 0);
	for (i = 0; i < MAXCONS; i++) {
	  if (who == nl_cons[i].proxy) {
		/* input from nl_cons[i] */
		nchars = read(nl_cons[i].fd, ibuf, IBUFSZ);
		assert(nchars > 0);
		bptr = ibuf;
		if (dev_arm(nl_cons[i].fd, nl_cons[i].rproxy, _DEV_EVENT_INPUT) == -1)
		  nl_error(3, "Error arming console");
		break;
	  }
	}
	if (i == MAXCONS) nlcg_receive(who);
  }
  nchars--;
  return(*bptr++);
}

/*
=Name nlcon_getch(): Get a character from console(s).
=Subject Nortlib Console Functions
=Synopsis
#include "nl_cons.h"

int nlcon_getch(void);

=Description

  The function takes keyboard input from one or more QNX consoles
  and returns the next character typed. There is no indication on
  which console the character was typed; the underlying
  assumption is that input from any console is equivalent.

=Returns

  The character entered.

=SeeAlso

  =Nortlib Console Functions=, =nlcg_receive=(),
  =Con_init_options=().

=End
*/
