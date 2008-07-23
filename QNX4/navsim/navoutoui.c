/* OUI output from the following packages:
   msghdr
   msghdr_init
   msg
   navout
   nl_error_init
*/
char *opt_string = "h:e:o:c:lvsy";
#include "oui.h"
#include "msg.h"
#include "navout.h"
#include "nortlib.h"
  void navout_init( int argc, char **argv )  {
	int optltr;
	int nargs;
	optind = 0;
	opterr = 0;
	while ((optltr = getopt(argc, argv, opt_string)) != -1) {
	  switch (optltr) {
		  case '?':
			nl_error(3, "Unrecognized Option -%c", optopt);
		  default:
			break;
	  }
	}
	nargs = argc - optind;
	if ( nargs < 1 ) nl_error( 3, "Must specify serial port" );
	else if ( nargs > 1 ) nl_error( 3, "Too many arguments" );
	init_serial_port(argv[optind]);
  }
  int (*nl_error)(int level, char *s, ...) = msg;

void oui_init_options(int argc, char **argv) {
  char *msg_hdr;
  msg_hdr = "navout";
  msg_init_options(msg_hdr, argc, argv);
  navout_init( argc, argv );
}

#ifdef __USAGE
%C	[options] <serial_port>
	-c <node>[,<attached name>] send msgs to another task; default: memo
	-e <error filename>
	-h <msg header>
	-l add a level of debug messages
	-o <device>[,<row>,<col>,<width>,<pass attr>,<warn>,<fail>,<debug>]
	-s no message sounds
	-v disable verbose to stderr
	-y disable system error message concatenation to messages

NAV serial stream simulator reads data from da_cache and writes
formatted serial stream to the specified device. If '-' is
specified, output is written to stdout. Minimum configuration
requires
  da_cache -S 1000-101B
#endif
