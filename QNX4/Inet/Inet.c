#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "Inet.h"
#include "nortlib.h"

/* Checks for errors and correct number of bytes read
   Returns 1 on any errors, with the appropriate response
   probably being to terminate
*/
int tmread( int socket, void *bfr, size_t nbytes ) {
  size_t nb;

  nb = read( socket, bfr, nbytes );
  if (nb == -1) {
	nl_error( 2, "Read returned error: %d", errno );
	return 1;
  } else if ( nb != nbytes ) {
	nl_error( 2, "Read returned %d, expected %d", nb, nbytes );
	return 1;
  }
  return 0;
}

/* tmreadline reads characters from the socket up to and
  including a newline character. If more than size
  characters are read, it is considered a fatal error.
  The data is checked to make sure all characters are printable,
  and the terminating newline is replaced with a NUL char.
*/
void tmreadline( int socket, char *buf, int size ) {
  while ( size > 0 ) {
	if ( tmread( socket, buf, 1 ) ) exit(1);
	if ( *buf == '\n' ) break;
	if ( !isprint(*buf) )
	  nl_error( 3, "Illegal character received during negotiation" );
	buf++;
	if ( --size == 0 )
	  nl_error( 3, "Buffer overflow during negotiation" );
  }
  *buf = '\0';
}

/* returns non-zero on error, aborts if size is in error */
int tmwrite( int socket, void *buf, int size ) {
  int rv = write( socket, buf, size );
  if (rv == -1) return 1;
  else if ( rv != size )
	nl_error( 3, "write returned %d, expected %d", rv, size );
  return 0;
}

void tmwritestr( int socket, char *str ) {
  if ( tmwrite( socket, str, strlen(str) ))
	nl_error( 3, "Write error in tmwritestr" );
}
