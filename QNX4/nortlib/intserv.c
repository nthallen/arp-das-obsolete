#include <errno.h>
#include <string.h>
#include "intserv.h"
#include "cltsrvr.h"

static Server_Def ISdef = { ISRV_NAME, 0, 0, 3, 0, 0, 0, 0 };

/* Returns -1 on error, 0 on success */
static int send_isrv( IntSrv_msg *buf ) {
  IntSrv_reply rep;
  
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
  
  buf.type = ISRV_INT_ATT;
  strncpy( buf.cardID, cardID, cardID_MAX );
  buf.cardID[ cardID_MAX - 1 ] = '\0';
  buf.u.region = region;
  buf.proxy = Proxy;
  buf.address = address;
  return send_isrv( &buf );
}

int IntSrv_Int_detach( char *cardID ) {
  IntSrv_msg buf;
  
  buf.type = ISRV_INT_DET;
  strncpy( buf.cardID, cardID, cardID_MAX );
  buf.cardID[ cardID_MAX - 1 ] = '\0';
  return send_isrv( &buf );
}
