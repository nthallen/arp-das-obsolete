#include "intserv.h"

/* Returns -1 on error, 0 on success */
static int send_isrv( IntSrv_msg *buf ) {
  static pid_t Isrv_pid = -1;
  IntSrv_reply rep;
  
  if ( Isrv_pid == -1 )
	Isrv_pid = qnx_name_locate( getnid(), ISRV_NAME, 0, 0 );
  if ( Isrv_pid == -1 ) {
	if ( nl_response )
	  nl_error( nl_response, "Unable to locate intserv" );
	return -1;
  } else {
	errno = EOK;
	if ( Send( pid, buf, &rep, sizeof(buf), sizeof(rep) ) != 0 ) {
	  if ( nl_response )
		nl_error( nl_response, "Error sending to intserv" );
	  return -1;
	} else if ( rep.status != EOK && nl_response ) {
	  errno = rep.status;
	  nl_error( nl_response, "Error %d from intserv", rep.status );
	  return -1;
	}
  }
  return 0;
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
