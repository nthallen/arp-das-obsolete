/* OUI output from the following packages:
   seteuid
   msghdr
   msghdr_init
   msg
   nl_error_init
   subbus
   subbus_reqd
   timerbd
*/
char *opt_string = "h:e:o:c:lvsyi:x";
#include "oui.h"
#include "msg.h"
#include "nortlib.h"
#include "subbus.h"
#include "tmrdrvr.h"
  int (*nl_error)(int level, char *s, ...) = msg;

void oui_init_options(int argc, char **argv) {
  char *msg_hdr;
  int subbus_id;
  msg_hdr = "TMR";
  msg_init_options(msg_hdr, argc, argv);
  subbus_id = load_subbus();
  if (subbus_id == 0)
	nl_error(3, "Subbus Library Not Resident");
  tmr_init_options( argc, argv );
}

#ifdef __USAGE
%C	[options]
	-c <node>[,<attached name>] send msgs to another task; default: memo
	-e <error filename>
	-h <msg header>
	-i IRQ specify IRQ number (1-15) selected on the system controller
	-l add a level of debug messages
	-o <device>[,<row>,<col>,<width>,<pass attr>,<warn>,<fail>,<debug>]
	-s no message sounds
	-v disable verbose to stderr
	-x Request a resident timerbd utility to terminate.
	-y disable system error message concatenation to messages
#endif
