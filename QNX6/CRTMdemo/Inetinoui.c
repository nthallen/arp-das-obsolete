/* OUI output from the following packages:
   msghdr
   msghdr_init
   msg
   Inetin
   dg
   nl_error_init
*/
char *opt_string = "h:e:o:c:lvsyn:j:";
#include "oui.h"
#include "dbr.h"
#include "msg.h"
#include "nortlib.h"

extern void initiate_connection(void);

void oui_init_options(int argc, char **argv) {
  initiate_connection();
  /* DG_init_options(argc, argv); */
}

#ifdef __USAGE
%C	[options]
	-c <node>[,<attached name>] send msgs to another task; default: memo
	-e <error filename>
	-h <msg header>
	-j <milliseconds> approximate regulator, delay between token data reads
	-l add a level of debug messages
	-n <number of clients> start ring after client initialisations
	-o <device>[,<row>,<col>,<width>,<pass attr>,<warn>,<fail>,<debug>]
	-s no message sounds
	-v disable verbose to stderr
	-y disable system error message concatenation to messages
Inetin requires the environment variables "RemEx" and
"RemoteHost" be defined. RemoteHost should be defined in
Experiment.config. RemEx will be set by the "doit" script based
on the value of "Experiment" before the latter is modified for
local use.
#endif
