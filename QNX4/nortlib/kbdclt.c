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

/* kbdclt_quit() is an init_options-kind-of-function which sets a
   keyboard client up to quit gracefully when an experiment
   terminates. It is referenced by kbdclt.oui.
*/

int kbdclt_quit( int argc, char **argv ) {
  int c;
  nid_t my_node, cis_node;
  pid_t cc_pid, cc_proxy = -1, ccr_proxy = -1;

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
  cc_pid = qnx_name_locate( cis_node,
			  nl_make_name( CMD_CTRL, 0 ), 0, 0 );
  if ( cc_pid == -1 )
	nl_error( -2, "Unable to locate " CMD_CTRL );
  PBdef.node = cis_node;
  if ( CltInit( &PBdef ) )
	nl_error( -2, "Unable to locate %s", PBdef.name );
  if ( cc_pid != -1 || PBdef.connected ) {
	cc_proxy = ccr_proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
	if ( ccr_proxy != -1 && cis_node != my_node)
	  ccr_proxy = qnx_proxy_rem_attach( cis_node, ccr_proxy );
	if ( ccr_proxy == -1 ) {
	  nl_error( 2, "Unable to create proxy for cmdctrl" );
	  return -1;
	}
  }
  if ( cc_pid != -1 ) {
	ccreg_type ccr;
	unsigned char rv;

    ccr.ccreg_byt = CCReg_MSG;
    ccr.min_dasc = ccr.max_dasc = ccr.min_msg = ccr.max_msg = 0;
    ccr.how_to_quit = PROXY_ON_QUIT;
    ccr.how_to_die = NOTHING_ON_DEATH;
	ccr.proxy = ccr_proxy;
	if ( Send( cc_pid, &ccr, &rv, sizeof( ccr ), sizeof( rv ) ) != 0 )
	  nl_error( 1, "Error Sending to CmdCtrl" );
	else if ( rv != DAS_OK )
	  nl_error( 1, "Error return from CmdCtrl: %d", rv );
  }
  if ( PBdef.connected ) {
	struct PBquit_s {
	  char hdr[4];
	  pid_t proxy;
	} PBquit;
	unsigned char rv;

	strcpy( PBquit.hdr, "pbQQ" );
	PBquit.proxy = ccr_proxy;
	CltSend( &PBdef, &PBquit, &rv, sizeof(PBquit), sizeof(rv) );
  }
  return cc_proxy;
}
