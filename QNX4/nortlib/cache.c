#include <errno.h>
#include <string.h>
#include "da_cache.h"
#include "cltsrvr.h"
#include "nortlib.h"
char rcsid_cache_c[] =
  "$Header$";

static Server_Def CAdef = { CACHE_NAME, 1, 0, 1, 0, 0, 0, 0 };

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

/*
=Name cache_read(): Read from D/A or SW Cache
=Subject Client/Server
=Subject Data Collection
=Name cache_write(): Write to D/A or SW Cache
=Subject Client/Server
=Subject Data Collection
=Name cache_quit(): Ask resident Cache driver to terminate
=Subject Client/Server
=Subject Data Collection

=Synopsis
#include "da_cache.h"

int cache_write( unsigned short addr, unsigned short value );
unsigned short cache_read( unsigned short addr );
int cache_quit( void );

=Description

These routines provide access to the da_cache driver.
cache_write() sets the value at the specified address. If the
address is within the hardware address range specified when the
driver was started, the value is masked with CACHE_HW_MASK and
then written out to the subbus address. If there is no
acknowledge from the hardware, the cached value is or-ed with
CACHE_NACK_MASK, which can be checked on read. (This mechanism is
currently only useful for hardware that supports less than 16
bits.)

cache_read() reads the stored value for the specified address. It
does not touch hardware, even for addresses within the hardware
range.

cache_quit() sends a quit request to a resident cache driver.

All of these functions use the standard nortlib client/server
routines, which means their behaviour can by modified by setting
=nl_response=, but the default response is set to "warn", so
you'll need to check your return codes.

=Returns

cache_write() and cache_quit() return the following status codes
defined in da_cache.h:

<UL>
<LI>CACHE_E_OK: Success
<LI>CACHE_E_UNKN: Invalid Command Type
<LI>CACHE_E_OOR: Invalid Address
<LI>CACHE_E_NACK: No acknowledge
<LI>CACHE_E_NOCACHE: Driver not found
</UL>

cache_read() returns the value associated with the specified
address. If the address is invalid, ~0 is returned. Values for
hardware addresses will included acknowledge information in the
CACHE_NACK_MASK bit.

=SeeAlso

<A HREF="../da_cache.html">da_cache</A>
driver.

=End
*/
