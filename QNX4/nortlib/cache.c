#include <errno.h>
#include <string.h>
#include "da_cache.h"
#include "cltsrvr.h"
#include "nortlib.h"
char rcsid_cache_c[] =
  "$Header$";

static Server_Def CAdef = { CACHE_NAME, 1, 0, 3, 0, 0, 0, 0 };

int cache_write( unsigned short a, unsigned short v ) {
  cache_msg msg;
  cache_rep rep;
  
  msg.header = CACHE_MSG;
  msg.type = CACHE_WRITE;
  msg.address = a;
  msg.value = v;
  if ( CltSend( &CAdef, &msg, &rep, sizeof(msg), sizeof(rep) ) ) {
	return CACHE_E_NOCACHE;
  } else return rep.status;
}

unsigned short cache_read( unsigned short a ) {
  cache_msg msg;
  cache_rep rep;
  
  msg.header = CACHE_MSG;
  msg.type = CACHE_READ;
  msg.address = a;
  if ( CltSend( &CAdef, &msg, &rep, sizeof(msg), sizeof(rep) ) ) {
	return ~0;
  } else return rep.value;
}

int cache_quit( void ) {
  cache_msg msg;
  cache_rep rep;
  
  msg.header = CACHE_MSG;
  msg.type = CACHE_QUIT;
  if ( CltSend( &CAdef, &msg, &rep, sizeof(msg), sizeof(rep) ) ) {
	return CACHE_E_NOCACHE;
  } else return rep.status;
}
