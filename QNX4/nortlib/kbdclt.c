#include <unistd.h>
#include <sys/kernel.h>
#include <sys/proxy.h>
#include <sys/name.h>
#include <stdlib.h>
#include <string.h>
#include "nortlib.h"
#include "cmdctrl.h"
#include "oui.h"
#include "cltsrvr.h"
char rcsid_kbdclt_c[] =
  "$Header$";

/* kbdclt_quit() is an init_options-kind-of-function which sets a
   keyboard client up to quit gracefully when an experiment
   terminates. It is referenced by kbdclt.oui.
*/

int kbdclt_quit( int argc, char **argv ) {
  int c;
  nid_t cis_node;

  cis_node = getnid();
  optind = 0;
  opterr = 0;
  if (argc > 0) do {
	c = getopt( argc, argv, opt_string);
	switch ( c ) {
	  case 'C':
		cis_node = atoi( optarg );
		break;
	  default:
		break;
	}
  } while ( c != -1 );
  return cc_quit_request( cis_node );
}

/*
=Name kbdclt_quit(): Setup Keyboard Client to quit gracefully
=Subject Client/Server
=Subject Startup

=Synopsis

#include "cltsrvr.h"
int kbdclt_quit( int argc, char **argv );

=Description

kbdclt_quit() is an init_options-kind-of-function which sets a
keyboard client up to quit gracefully when an experiment
terminates. It is referenced by kbdclt.oui. This function invokes
a lower-level setup via the function =cc_quit_request=(). That
function allocates an empty proxy and passes it to CmdCtrl or
PBreg. The proxy will be triggered when the appropriate server
receives a quit command.

=Returns

On success, the PID of the quit proxy is returned. On failure, if
=nl_response= is set low enough to prevent an abort, -1 is
returned.

=SeeAlso

=cc_quit_request=().

=End
*/
