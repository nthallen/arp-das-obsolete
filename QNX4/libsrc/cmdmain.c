/* cmdmain.c A main program for interactive commanding.
 * $Log$
 * Revision 1.2  1993/03/01  18:39:16  nort
 * Interface function name change
 *
 * Revision 1.1  1993/01/09  15:20:07  nort
 * Initial revision
 *
 */
#include <signal.h>
#include <stdlib.h>
#include "nl_cons.h"
#include "msg.h"
#include "cc.h"
#include "cmdalgo.h"
#include "subbus.h"
static char rcsid[] = "$Id$";

char *opt_string = OPT_MSG_INIT OPT_CON_INIT;
int (*nl_error)(unsigned int level, char *s, ...) = msg;

void quit_catcher(int sig) { _exit( sig != SIGQUIT ); }

int main(int argc, char **argv) {
  msg_init_options("Cmd", argc, argv);
  Con_init_options(argc, argv);
  load_subbus();
  atexit(nlcon_close);
  {
	int nlrsp;
	  
	nlrsp = set_response(NLRSP_QUIET);
	if (find_CC(0) != -1) {
	  set_response(NLRSP_DIE);
	  cc_init(0, 0, 0, 0, QUIT_ON_QUIT, NOTHING_ON_DEATH, NULL);
	  if (signal(SIGQUIT, quit_catcher) == SIG_ERR)
		msg(MSG_EXIT_ABNORM, "Unable to attach SIGINT");
	} else set_response(nlrsp);
  }
  
  cmd_interact();

  return(0);
}
