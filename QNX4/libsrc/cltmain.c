/* cltmain.c A main program for interactive commanding as a client.
 * $Log$
 * Revision 1.1  1993/03/01  18:40:18  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include "nortlib.h"
#include "nl_cons.h"
#include "msg.h"
#include "cmdalgo.h"
static char rcsid[] = "$Id$";

char *opt_string = OPT_MSG_INIT OPT_CON_INIT OPT_CIC_INIT;
int (*nl_error)(unsigned int level, char *s, ...) = msg;

int main(int argc, char **argv) {
  msg_init_options("Cmd", argc, argv);
  Con_init_options(argc, argv);
  cic_options(argc, argv, "cic");
  atexit(nlcon_close);
  cmd_interact();
  return(0);
}
