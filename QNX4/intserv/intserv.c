/* intserv.c
   Interrupt handler for Syscon104
*/
#include <errno.h> /* for errno */
#include <string.h> /* for strerror */
#include <unistd.h> /* for getnid() */
#include <sys/kernel.h>
#include <sys/name.h>
#include "nortlib.h"
#include "oui.h"
#include "intserv.h"
#include "internal.h"
#include "subbus.h"

int intserv_quit = 0;

int main( int argc, char **argv ) {
  int name_id, subbus_id, done = 0;
  
  oui_init_options( argc, argv );
  if ( intserv_quit ) {
	pid_t isrv;
	
	/* request running server to quit */
	isrv = qnx_name_locate( getnid(), ISRV_NAME, 0, 0 );
	if ( isrv == -1 ) {
	  nl_error( 3, "Quit request failed" );
	} else {
	  msg_t type;
	  type = ISRV_QUIT;
	  errno = EOK;
	  if ( Send( isrv, &type, &type, sizeof( type ), sizeof( type ) )
			  != 0 || type != EOK )
		nl_error( 3, "Error requesting quit" );
	  nl_error( 0, "Quit request acknowledged" );
	}
	return 0;
  }

  subbus_id = load_subbus();
  expint_init();
  name_id = qnx_name_attach( 0, ISRV_NAME );
  if ( name_id == -1 )
	nl_error( 3, "Error attaching name" );
  errno = EOK;

  while ( ! done ) {
	pid_t who;
	IntSrv_msg buf;
	IntSrv_reply rep;

	who = Receive( 0, &buf, sizeof( buf ) );
	if ( who == -1 ) nl_error( 1, "Receive error" );
	else if ( who == expint_proxy ) {
	  service_expint();
	} else {
	  rep.status = EOK;
	  switch ( buf.type ) {
		case ISRV_QUIT:
		  errno = EOK;
		  nl_error( 0, "Quit request received" );
		  done = 1;
		  break;
		case ISRV_INT_ATT:
		  expint_attach( who, buf.cardID, buf.address, buf.u.region, 
						buf.proxy, &rep );
		  break;
		case ISRV_INT_DET:
		  expint_detach( who, buf.cardID, &rep );
		  break;
		case ISRV_IRQ_ATT:
		case ISRV_IRQ_DET:
		default:
		  rep.status = ENOSYS;
		  break;
	  }
	  if ( Reply( who, &rep, sizeof( rep ) ) != 0 )
		nl_error( 1, "Reply error" );
	}
  }
  errno = EOK;
  expint_reset();
  qnx_name_detach( 0, name_id );
  nl_error( 0, "Terminating" );
  return 0;
}
