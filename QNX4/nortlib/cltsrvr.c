/* cltsrvr.c Provides general routines for interfacing to a 
  server. I may wish to change this approach as I use it
  and require more flexibility.
  
  CltInit() and CltSend() deal with errors using the minimum 
  value of nl_response and def->response. i.e. a client which 
  normally dies when the server isn't present can be made to warn 
  only via set_response(1).

  This is something of an experiment to see if I can combine
  common practice for a number of client/server APIs and
  minimize the amount of code for each API. Here are the APIs 
  which might benefit from using this interface:

	  intserv.c (already migrated)
	  rtgapi.c (tries to re-establish connection every 20 seconds)
	  cic.c (dies if connection is lost)
	  colsend.c (doesn't die, doesn't re-establish connection)
	  ttclient.c (attempts to re-establish connection)
	  timerbd
	  subbus
	  idx64.c (there from the start)
*/
#include <errno.h>
#include <unistd.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include "nortlib.h"
#include "cltsrvr.h"
char rcsid_cltsrvr_c[] =
  "$Header$";

/* Returns 0 on success, -1 otherwise */
int CltInit( Server_Def *def ) {
  char *fname;
  int rv, resp;

  if ( def->connected ) return 0;
  if ( def->global ) def->node = 0;
  else if ( def->node == 0 ) def->node = getnid();

  resp = set_response( def->response < nl_response ? 
						def->response : nl_response );

  if ( def->expand )
	fname = nl_make_name( def->name, def->global );
  else fname = def->name;

  if ( fname == 0 ) rv = -1;
  else {
	def->pid = qnx_name_locate( def->node, fname, 0, 0 );
	if ( def->pid == -1 ) {
	  if ( def->disconnected == 0 ) {
		if ( nl_response )
		  nl_error( nl_response, "Unable to locate %s", fname );
	  }
	  def->disconnected = 1;
	  rv = -1;
	} else {
	  def->connected = 1;
	  def->disconnected = 0;
	  rv = 0;
	}
  }
  set_response( resp );
  return rv;
}

int CltSend( Server_Def *def, void *smsg, void *rmsg,
			  int sbytes, int rbytes ) {
  struct _mxfer_entry sx, rx;
  _setmx( &sx, smsg, sbytes );
  _setmx( &rx, rmsg, rbytes );
  return CltSendmx( def, 1, 1, &sx, &rx );
}

/* returns 0 on success, -1 otherwise
   If/when I need CltSendmx, simply modify this to be that
   and then provide the thin cover for CltSend()
 */
int CltSendmx( Server_Def *def, unsigned sparts, unsigned rparts,
           struct _mxfer_entry *smsg,
		   struct _mxfer_entry *rmsg ) {
  int resp;
  
  if ( ! def->connected && CltInit( def ) != 0 )
	return -1;
  resp = ( def->response < nl_response ) ? def->response : 
											nl_response;
  while ( Sendmx( def->pid, sparts, rparts, smsg, rmsg ) != 0 ) {
	if ( errno == ESRCH ) {
	  def->connected = 0;
	  def->disconnected = 1;
	  if ( resp )
		nl_error( resp, "Lost contact with %s", def->name );
	  if ( CltInit( def ) != 0 ) return -1;
	} else if ( errno != EINTR ) {
	  if ( resp )
		nl_error( resp, "Error sending to %s", def->name );
	  return -1;
	}
  }
  return 0;
}
/*
=Name CltSend(): Send message to a Server
=Subject Client/Server
=Name CltSendmx(): Sendmx message to a Server
=Subject Client/Server
=Name CltInit(): Initialize communication to a Server
=Subject Client/Server
=Subject Startup
=Synopsis
#include "cltsrvr.h"
int CltInit( Server_Def *def );
int CltSend( Server_Def *def, void *smsg, void *rmsg, int sbytes, int rbytes );
int CltSendmx( Server_Def *def, unsigned sparts, unsigned rparts,
           struct _mxfer_entry *smsg,
           struct _mxfer_entry *rmsg );

=Description

  CltInit() and CltSend() provide a general approach to writing
  Client/Server applications. Based on a configuration structure
  for a specific server, these functions perform many of the
  routine tasks involved in maintaining communication between the
  client and the server. They will locate the server, report
  errors if the server disappears, and even attempt to reconnect
  to the server in case it is restarted.<P>
  
  CltInit() is charged with locating the server (if it has not
  already been located) without actually sending it any messages.
  It is not necessary to call CltInit() before calling CltSend(),
  but some applications will prefer to report problems earlier
  rather than later.<P>
  
  CltSend() is a cover for the QNX Send() function. It verifies
  only that the transaction was successful, but does not attempt
  to interpret the reply data.<P>
  
  CltSend() will restart transactions which are interrupted by
  signals. If you wish to have signals abort a CltSend(), you
  will need to use sigsetjmp() and siglongjmp().<P>

  CltInit() and CltSend() deal with errors using the minimum 
  value of =nl_response= and def->response. i.e. a client which 
  normally dies when the server isn't present can be made to warn 
  only via set_response(1). Similarly, even if nl_response is set
  to 3, a particular server may never warrant a fatal error, so
  def->response may be set to 1 or 2 (or even 0) to allow for
  non-fatal responses.<P>

=Returns
  Both functions return zero on success and -1 otherwise.

=SeeAlso
  =Client/Server= functions.

=End
*/
