/* cis.c Defines functions used by Command Interpreter Server
 * $Log$
 * Revision 1.5  1995/10/31  19:21:42  nort
 * Support for proxy_on_quit from cmdctrl
 *
 * Revision 1.4  1994/02/16  02:13:07  nort
 * Added cis_initialize() call
 * Added support for CMDINTERP_SEND_QUIET
 * Move CMDINTERP_TEST so it won't write a message
 *
 * Revision 1.3  1993/09/15  19:26:01  nort
 * Using nl_make_name()
 *
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
#include <sys/proxy.h>
#include "nortlib.h"
#include "cmdalgo.h"
#include "cmdctrl.h"
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
  pid_t who, quit_proxy = -1;
  ci_ver v;
  unsigned short rv;
  char *name;

  cis_initialize();
  name = nl_make_name(CMDINTERP_NAME, 0);
  name_id = qnx_name_attach(0, name);
  if (name_id == -1)
	nl_error(3, "Unable to attach name %s", name);

  { /* Set up quit_proxy with cmdctrl if possible */
	pid_t cc_pid;
	int resp;
	
	resp = set_response( 0 );
	cc_pid = find_CC( 0 );
	set_response( resp );
	if ( cc_pid == -1 )
	  nl_error( -2, "Unable to locate cmdctrl for quit proxy" );
	else {
	  ccreg_type ccr;
	  unsigned char rv;

	  ccr.ccreg_byt = CCReg_MSG;
	  ccr.min_dasc = ccr.max_dasc = ccr.min_msg = ccr.max_msg = 0;
	  ccr.how_to_quit = PROXY_ON_QUIT;
	  ccr.how_to_die = NOTHING_ON_DEATH;
	  quit_proxy = ccr.proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
	  if ( ccr.proxy == -1 )
		nl_error( 2, "Unable to create proxy for cmdctrl" );
	  else {
		if ( Send( cc_pid, &ccr, &rv, sizeof( ccr ), sizeof( rv ) ) != 0 )
		  nl_error( 1, "Error Sending to CmdCtrl" );
		else if ( rv != DAS_OK )
		  nl_error( 1, "Error return from CmdCtrl: %d", rv );
	  }
	}
  }

  /* Basic Receive() loop */  
  for (;;) {
	who = Receive( 0, &cim, sizeof(cim) );
	if (who == -1) nl_error(0, "Receive gave errno %d", errno);
	else if ( who == quit_proxy ) {
	  nl_error( 0, "Received quit proxy from cmdctrl" );
	  break;
	} else switch (cim.msg_type) {
	  case CMDINTERP_QUERY:
		v.msg_type = 0;
		strncpy(v.version, ci_version, CMD_VERSION_MAX);
		v.version[CMD_VERSION_MAX-1] = '\0';
		Reply(who, &v, sizeof(v));
		continue;
	  case CMDINTERP_SEND:
	  case CMDINTERP_SEND_QUIET:
		{ int len;

		  len = strlen(cim.command);
		  if (len > 0 && cim.command[len-1] == '\n') len--;
		  nl_error(cim.msg_type == CMDINTERP_SEND_QUIET ? -2 : 0,
				  "%s: %*.*s", cim.prefix, len, len, cim.command);
		}
	  case CMDINTERP_TEST:
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
