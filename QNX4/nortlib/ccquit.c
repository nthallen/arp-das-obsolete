#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/proxy.h>
#include "nortlib.h"
#include "cltsrvr.h"
#include "cmdctrl.h"

static Server_Def CCdef = { CMD_CTRL, 1, 0, 1, 0, 0, 0, 0 };

pid_t cc_quit_request( nid_t cc_node ) {
  ccreg_type ccr;
  unsigned char rv;
  pid_t cc_proxy;
  nid_t my_node;

  my_node = getnid();
  if ( cc_node == 0 ) cc_node = my_node;
  CCdef.node = cc_node;
  ccr.ccreg_byt = CCReg_MSG;
  ccr.min_dasc = ccr.max_dasc = ccr.min_msg = ccr.max_msg = 0;
  ccr.how_to_quit = PROXY_ON_QUIT;
  ccr.how_to_die = NOTHING_ON_DEATH;
  cc_proxy = ccr.proxy = qnx_proxy_attach( 0, NULL, 0, -1 );
  if ( ccr.proxy != -1 && cc_node != my_node)
	ccr.proxy = qnx_proxy_rem_attach( cc_node, ccr.proxy );
  if ( ccr.proxy == -1 )
	nl_error( 2, "Unable to create proxy for cmdctrl" );
  else {
	if ( CltSend( &CCdef, &ccr, &rv, sizeof( ccr ), sizeof( rv ) ) != 0 )
	  nl_error( 1, "Error Sending to CmdCtrl" );
	else if ( rv != DAS_OK )
	  nl_error( 1, "Error return from CmdCtrl: %d", rv );
  }
  return cc_proxy;
}
