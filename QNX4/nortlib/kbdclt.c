#include <unistd.h>
#include <sys/kernel.h>
#include <sys/proxy.h>
#include <stdlib.h>
#include "nortlib.h"
#include "cmdctrl.h"
#include "oui.h"

/* kbdclt_quit() is an init_options-kind-of-function which sets a
   keyboard client up to quit gracefully when an experiment
   terminates. It is referenced by kbdclt.oui.
*/

int kbdclt_quit( int argc, char **argv ) {
  int c;
  int resp;
  nid_t my_node, cis_node;
  pid_t cc_pid;

  my_node = cis_node = getnid();
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
  resp = set_response( 0 );
  cc_pid = nl_find_name( cis_node, nl_make_name( CMD_CTRL, 0 ) );
  if ( cc_pid != -1 ) {
	ccreg_type ccr;
	unsigned char rv;

    ccr.ccreg_byt = CCReg_MSG;
    ccr.min_dasc = ccr.max_dasc = ccr.min_msg = ccr.max_msg = 0;
    ccr.how_to_quit = PROXY_ON_QUIT;
    ccr.how_to_die = NOTHING_ON_DEATH;
	ccr.proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
	if ( ccr.proxy != -1 && cis_node != my_node)
	  ccr.proxy = qnx_proxy_rem_attach( cis_node, ccr.proxy );
	if ( ccr.proxy == -1 )
	  nl_error( 2, "Unable to create proxy for cmdctrl" );
	else {
	  if ( Send( cc_pid, &ccr, &rv, sizeof( ccr ), sizeof( rv ) ) != 0 )
		nl_error( 1, "Error Sending to CmdCtrl" );
	  else if ( rv != DAS_OK )
		nl_error( 1, "Error return from CmdCtrl: %d", rv );
	}
  } else nl_error( -2, "Unable to locate " CMD_CTRL );
  set_response( resp );
  return cc_pid;
}
