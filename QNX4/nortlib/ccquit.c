#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/proxy.h>
#include "nortlib.h"
#include "cltsrvr.h"
#include "cmdctrl.h"
char rcsid_ccquit_c[] =
  "$Header$";

static Server_Def CCdef = { CMD_CTRL, 1, 0, 1, 0, 0, 0, 0 };

pid_t cc_quit_request( nid_t cc_node ) {
  unsigned char rv;
  pid_t cc_proxy, ccr_proxy;
  nid_t my_node;

  my_node = getnid();
  if ( cc_node == 0 ) cc_node = my_node;
  CCdef.node = PBdef.node = cc_node;
  cc_proxy = ccr_proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
  if ( ccr_proxy != -1 && cc_node != my_node)
	ccr_proxy = qnx_proxy_rem_attach( cc_node, ccr_proxy );
  if ( ccr_proxy == -1 )
	nl_error( 2, "Unable to create remote proxy for quit" );
  else {
	if ( !CltInit(&CCdef) ) {
	  ccreg_type ccr;
	  ccr.ccreg_byt = CCReg_MSG;
	  ccr.min_dasc = ccr.max_dasc = ccr.min_msg = ccr.max_msg = 0;
	  ccr.how_to_quit = PROXY_ON_QUIT;
	  ccr.how_to_die = NOTHING_ON_DEATH;
	  ccr.proxy = ccr_proxy;
	  if ( CltSend( &CCdef, &ccr, &rv, sizeof( ccr ), sizeof( rv ) ) != 0 )
		nl_error( 1, "Error Sending to CmdCtrl" );
	  else if ( rv != DAS_OK )
		nl_error( 1, "Error return from CmdCtrl: %d", rv );
	} else if ( !CltInit( &PBdef ) ) {
	  struct PBquit_s {
		char hdr[4];
		pid_t proxy;
	  } PBquit;
	  unsigned char rv;

	  strcpy( PBquit.hdr, "pbQQ" );
	  PBquit.proxy = ccr_proxy;
	  CltSend( &PBdef, &PBquit, &rv, sizeof(PBquit), sizeof(rv) );
	} else nl_error( 1, "Quit proxy not registered" );
  }
  return cc_proxy;
}

/*
=Name cc_quit_request(): Request quit proxy from CmdCtrl or PBreg
=Subject Client/Server
=Subject Startup

=Synopsis
#include "cltsrvr.h"

pid_t cc_quit_request( nid_t cc_node );

=Description

cc_quit_request() attempts to contact either the CmdCtrl or PBreg
processes to request that a quit proxy be sent at the appropriate
time.

=Returns

On success, the PID of the quit proxy is returned. On failure, if
=nl_response= is set low enough to prevent an abort, -1 is returned.

=SeeAlso
=kbdclt_quit=().
=End
*/

