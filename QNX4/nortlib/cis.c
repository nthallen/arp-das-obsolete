/* cis.c Defines functions used by Command Interpreter Server
 * $Log$
 * Revision 1.2  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.1  1993/02/11  03:19:42  nort
 * Initial revision
 *
 */
#include <string.h>
#include <errno.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include "nortlib.h"
#include "cmdalgo.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

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
  char *name;
  
  name = nl_make_name(CMDINTERP_NAME);
  name_id = qnx_name_attach(0, name);
  if (name_id == -1)
	nl_error(3, "Unable to attach name %s", name);
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
		{ int len;

		  len = strlen(cim.command);
		  if (len > 0 && cim.command[len-1] == '\n') len--;
		  nl_error(0, "%s: %*.*s", cim.prefix, len, len, cim.command);
		}
		cmd_init();
		rv = cmd_batch(cim.command, cim.msg_type == CMDINTERP_TEST);
		Reply(who, &rv, sizeof(rv));
		if (rv != CMDREP_QUIT || cim.msg_type == CMDINTERP_TEST)
		  continue;
		break;
	  case CMDINTERP_QUIT:
		rv = CMDREP_QUIT;
		Reply(who, &rv, sizeof(rv));
		break;
	}
	break;
  }
  qnx_name_detach(0, name_id);
}
