#include <errno.h>
#include <string.h>
#include "intserv.h"
#include "cltsrvr.h"
#include "nortlib.h"
char rcsid_intserv_c[] =
  "$Header$";

static Server_Def ISdef = { ISRV_NAME, 0, 0, 3, 0, 0, 0, 0 };

/* Returns -1 on error, 0 on success */
static int send_isrv( IntSrv_msg *buf, msg_t type, char *cardID ) {
  IntSrv_reply rep;
  
  buf->type = type;
  strncpy( buf->cardID, cardID, cardID_MAX );
  buf->cardID[ cardID_MAX - 1 ] = '\0';
  if ( CltSend( &ISdef, buf, &rep, sizeof(IntSrv_msg),
		  sizeof(rep) )	== 0 ) {
	if ( rep.status == EOK ) return 0;
	errno = rep.status;
  }
  return -1;
}

int IntSrv_Int_attach( char *cardID, unsigned short address,
						int region, pid_t Proxy ) {
  IntSrv_msg buf;
  
  buf.u.region = region;
  buf.proxy = Proxy;
  buf.address = address;
  return send_isrv( &buf, ISRV_INT_ATT, cardID );
}

/* I'm going to make the response level 1 for detach, even though
   the mode as a whole should be 3
*/
int IntSrv_Int_detach( char *cardID ) {
  IntSrv_msg buf;
  int resp, rv;
  
  resp = set_response( 1 );
  rv = send_isrv( &buf, ISRV_INT_DET, cardID );
  set_response( resp );
  return rv;
}

int IntSrv_IRQ_attach( char *cardID, int IRQ, pid_t Proxy ) {
  IntSrv_msg buf;
  
  buf.u.irq = IRQ;
  buf.proxy = Proxy;
  return send_isrv( &buf, ISRV_IRQ_ATT, cardID );
}

int IntSrv_IRQ_detach( char *cardID, int IRQ ) {
  IntSrv_msg buf;
  int resp, rv;
  
  buf.u.irq = IRQ;
  resp = set_response( 1 );
  rv = send_isrv( &buf, ISRV_IRQ_DET, cardID );
  set_response( resp );
  return rv;
}

/*
=Name IntSrv_Int_attach(): Attach proxy to external interrupt
=Subject Interrupt Services
=Name IntSrv_Int_detach(): Detach proxy from external interrupt
=Subject Interrupt Services
=Name IntSrv_IRQ_attach(): Attach proxy to interrupt
=Subject Interrupt Services
=Name IntSrv_IRQ_detach(): Detach proxy from interrupt
=Subject Interrupt Services

=Synopsis
#include "intserv.h"

int IntSrv_Int_attach( char *cardID, unsigned short address,
						int region, pid_t Proxy );
int IntSrv_Int_detach( char *cardID );
int IntSrv_IRQ_attach( char *cardID, int IRQ, pid_t Proxy );
int IntSrv_IRQ_detach( char *cardID, int IRQ );

=Description

IntSrv_Int_attach() and IntSrv_Int_detach() provide the API
support for the interrupt architecture of the "Subbus64" bus. All
boards on this bus share a common interrupt line. To determine
which board asserted an interrupt, intserv reads from the
interrupt acknowledge address. Each board is dynamically assigned
one of the 8 acknowledge lines, and will assert that line during
interrupt acknowledge if an interrupt is pending.

IntSrv_Int_attach() configures the card at the specified address
and associates the Proxy with its interrupt. CardID is a string
which uniquely identifies the card which is being configured.
If the requesting process dies without detaching the interrupt
and is then restarted, intserv will detect that a request for the
same card has been received and figure out what the problem is.

IntSrv_Int_detach() disables the card's interrupt.

IntSrv_IRQ_attach() attaches a proxy to the specified IRQ. This
allows a non-privileged process to service interrupts indirectly.

IntSrv_IRQ_detach() detaches the proxy from the specified IRQ.

=Returns

Zero on success.

=SeeAlso

<A HREF="http://www.arp.harvard.edu/eng/das/manuals/idx64/idx64.html#SBIF">Indexer 64
Implementation</A>

=End
*/
