/* srvmain.c A main program for command servers.
 * $Log$
 * Revision 1.1  1993/05/28  20:06:09  nort
 * Initial revision
 *
 * Revision 1.1  1993/03/01  18:40:26  nort
 * Initial revision
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include "nortlib.h"
#include "msg.h"
#include "cmdalgo.h"
#include "subbus.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

char *opt_string = OPT_MSG_INIT;
int (*nl_error)(unsigned int level, char *s, ...) = msg;
void cis_init(void);

int main(int argc, char **argv) {
  seteuid(getuid());
  msg_init_options("Cmd", argc, argv);
  BEGIN_MSG;
  load_subbus();
  ci_server();
  DONE_MSG;
  return(0);
}
