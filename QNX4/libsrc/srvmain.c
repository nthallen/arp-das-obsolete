/* srvmain.c A main program for command servers.
 * $Log$
 * Revision 1.1  1993/03/01  18:40:26  nort
 * Initial revision
 *
 */
#include "nortlib.h"
#include "msg.h"
#include "cmdalgo.h"
#include "subbus.h"
static char rcsid[] = "$Id$";

char *opt_string = OPT_MSG_INIT;
int (*nl_error)(unsigned int level, char *s, ...) = msg;

int main(int argc, char **argv) {
  msg_init_options("Cmd", argc, argv);
  load_subbus();
  ci_server();
  return(0);
}
