/* cis.c Defines functions used by Command Interpreter Server
 * $Log$
 */
#include <string.h>
#include <errno.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include "nortlib.h"
#include "cmdalgo.h"
static char rcsid[] = "$Id$";

/* ci_server() does all the work for a command server. It does
   not return until cmd_batch returns a CMDREP_QUIT or it receives
   a CMDINTERP_QUIT message.
   It registers the CMDINTERP_NAME
   Loops to Receive messages. For each received command,
   calls cmd_init() and cmd_batch. If needed, a hook can
   be added for other messages.
*/
void ci_server(void) {
  int name_id;
  ci_msg cim;
  pid_t who;
  ci_ver v;
  unsigned short rv;
  
  name_id = qnx_name_attach(0, CMDINTERP_NAME);
  if (name_id == -1)
	nl_error(3, "Unable to attach name " CMDINTERP_NAME);
  for (;;) {
	who = Receive(0, &cim, sizeof(cim));
	if (who == -1) nl_error(0, "Receive gave errno %d", errno);
	else switch (cim.msg_type) {
	  case CMDINTERP_QUERY:
		v.msg_type = 0;
		strncpy(v.version, ci_version, CMD_VERSION_MAX);
		v.version[CMD_VERSION_MAX-1] = '\0';
		Reply(who, &v, sizeof(v));
		continue;
	  case CMDINTERP_SEND:
	  case CMDINTERP_TEST:
		cmd_init();
		rv = cmd_batch(cim.command, cim.msg_type == CMDINTERP_TEST);
		nl_error(0, "%s: %s", cim.prefix, cim.command);
		Reply(who, &rv, sizeof(rv));
		if (rv != CMDREP_QUIT || cim.msg_type == CMDINTERP_TEST)
		  continue;
		break;
	  case CMDINTERP_QUIT:
		rv = CMDREP_QUIT;
		Reply(who, &rv, sizeof(rv));
		break;
	}
  }
  qnx_name_detach(0, name_id);
}
