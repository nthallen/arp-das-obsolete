/* cltsrvr.c Provides general routines for interfacing to a 
  server. I may wish to change this approach as I use it
  and require more flexibility.
  
  CltInit() and CltSend() deal with errors using the minimum 
  value of nl_response and def->response. i.e. a client which 
  normally dies when the server isn't present can be made to warn 
  only via set_response(1). However, CltInit() (called by 
  CltSend()) calls nl_make_name() [ and I reserve the right to
  call other nortlib routines in the future ] which respond 
  strictly to nl_response, so if you really want to be 
  bulletproof and handle all your errors, you would need to 
  set_response() to a low value even for servers with a low 
  response code.
  
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
*/
#include <errno.h>
#include <unistd.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include "nortlib.h"
#include "cltsrvr.h"

/* Returns 0 on success, -1 otherwise */
int CltInit( Server_Def *def ) {
  char *fname;

  if ( def->connected ) return 0;
  if ( def->global ) def->node = 0;
  else if ( def->node == 0 ) def->node = getnid();
  if ( def->expand ) {
	fname = nl_make_name( def->name, def->global );
	if ( fname == 0 ) return -1;
  } else fname = def->name;
  def->pid = qnx_name_locate( def->node, fname, 0, 0 );
  if ( def->pid == -1 ) {
	if ( def->disconnected == 0 ) {
	  int resp;

	  resp = ( def->response < nl_response ) ? def->response : 
												nl_response;
	  if ( resp )
		nl_error( resp, "Unable to locate %s", fname );
	}
	def->disconnected = 1;
	return -1;
  } else {
	def->connected = 1;
	def->disconnected = 0;
	return 0;
  }
}

/* returns 0 on success, -1 otherwise
   If/when I need CltSendmx, simply modify this to be that
   and then provide the thin cover for CltSend()
 */
int CltSend( Server_Def *def, void *smsg, void *rmsg,
			  int sbytes, int rbytes ) {
  int resp;
  
  if ( ! def->connected && CltInit( def ) != 0 )
	return -1;
  resp = ( def->response < nl_response ) ? def->response : 
											nl_response;
  while ( Send( def->pid, smsg, rmsg, sbytes, rbytes ) != 0 ) {
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
