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
