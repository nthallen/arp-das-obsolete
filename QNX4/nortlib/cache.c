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
  msg.range_value = v;
  if ( CltSend( &CAdef, &msg, &rep, sizeof(msg), sizeof(rep) ) ) {
	return CACHE_E_NOCACHE;
  } else return rep.status;
}

int cache_writev( unsigned short a, unsigned short n_bytes, char *data ) {
  cache_msg msg;
  cache_rep rep;
  struct _mxfer_entry sx[2], rx;
  
  msg.header = CACHE_MSG;
  msg.type = CACHE_WRITEV;
  msg.address = a;
  msg.range_value = n_bytes;
  _setmx( &sx[0], &msg, sizeof(cache_msg) );
  _setmx( &sx[1], data, n_bytes );
  _setmx( &rx, &rep, sizeof(cache_rep) );
  if ( CltSendmx( &CAdef, 2, 1, sx, &rx ) ) {
	return CACHE_E_NOCACHE;
  } else return rep.status;
}

int cache_lwrite( unsigned short a, unsigned long v ) {
  return
	cache_writev( a, sizeof(unsigned long), (char *)&v );
}

int cache_fwrite( unsigned short a, float f ) {
  return
	cache_writev( a, sizeof(float), (char *)&f );
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

unsigned short cache_readv( unsigned short a, unsigned short l,
		  char *data ) {
  cache_msg msg;
  cache_rep rep;
  struct _mxfer_entry sx, rx[2];
  
  msg.header = CACHE_MSG;
  msg.type = CACHE_READV;
  msg.address = a;
  msg.range_value = l;
  _setmx( &sx, &msg, sizeof(cache_msg) );
  _setmx( &rx[0], &rep, sizeof(cache_rep) );
  _setmx( &rx[1], data, l );
  if ( CltSendmx( &CAdef, 1, 2, &sx, rx ) ) {
	return ~0;
  } else return rep.status;
}

unsigned long cache_lread( unsigned short a ) {
  unsigned long l;
  if ( cache_readv( a, sizeof(unsigned long), (char *)&l ) ) {
	return ~0L;
  } else return l;
}

float cache_fread( unsigned short a ) {
  float f;
  if ( cache_readv( a, sizeof(float), (char *)&f ) ) {
	return ~0L;
  } else return f;
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
=Name cache_lread(): Read long from D/A or SW Cache
=Subject Client/Server
=Subject Data Collection
=Name cache_fread(): Read float from D/A or SW Cache
=Subject Client/Server
=Subject Data Collection
=Name cache_write(): Write to D/A or SW Cache
=Subject Client/Server
=Subject Data Collection
=Name cache_writev(): Write to D/A or SW Cache
=Subject Client/Server
=Subject Data Collection
=Name cache_lwrite(): Write long to D/A or SW Cache
=Subject Client/Server
=Subject Data Collection
=Name cache_fwrite(): Write float to D/A or SW Cache
=Subject Client/Server
=Subject Data Collection
=Name cache_quit(): Ask resident Cache driver to terminate
=Subject Client/Server
=Subject Data Collection

=Synopsis
#include "da_cache.h"

int cache_write( unsigned short addr, unsigned short value );
int cache_writev( unsigned short a, unsigned short n_bytes,
                  char *data );
int cache_lwrite( unsigned short addr, unsigned long value );
int cache_fwrite( unsigned short addr, float value );
unsigned short cache_read( unsigned short addr );
unsigned short cache_readv( unsigned short a, unsigned short l,
                            char *data );
unsigned long cache_lread( unsigned short addr );
float cache_fread( unsigned short addr );
int cache_quit( void );

=Description

These routines provide access to the da_cache driver.
cache_*write*() sets the value at the specified address(es). If
the address is within the hardware address range specified when
the driver was started, the value is masked with CACHE_HW_MASK
and then written out to the subbus address. If there is no
acknowledge from the hardware, the cached value is or-ed with
CACHE_NACK_MASK, which can be checked on read. The l and f
versions call cache_writev() with appropriate arguments.

cache_*read*() reads the stored value for the specified address. It
does not touch hardware, even for addresses within the hardware
range. The l and f versions call cache_readv() with appropriate
arguments.

cache_quit() sends a quit request to a resident cache driver.

All of these functions use the standard nortlib client/server
routines, which means their behaviour can by modified by setting
=nl_response=, but the default response is set to "warn", so
you'll need to check your return codes.

=Returns

cache_*write*(), cache_readv() and cache_quit() return the
following status codes defined in da_cache.h:

<UL>
<LI>CACHE_E_OK: Success
<LI>CACHE_E_UNKN: Invalid Command Type
<LI>CACHE_E_OOR: Invalid Address
<LI>CACHE_E_NACK: No acknowledge
<LI>CACHE_E_NOCACHE: Driver not found
</UL>

cache_*read() returns the value associated with the specified
address. If the address is invalid, ~0 is returned. Values for
hardware addresses will include acknowledge information in the
CACHE_NACK_MASK bit. It is fairly easy to confuse the invalid
return values with data values, particularly for the l and f
versions.

=SeeAlso

<A HREF="../da_cache.html">da_cache</A> driver.

=End
*/
