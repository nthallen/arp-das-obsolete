#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "nortlib.h"

#define BUFSIZE 80

int main( int argc, char **argv ) {
  int fd, nb;
  char buf[BUFSIZE+1];

  if ( argc < 2 ) 
    nl_error( 3, "Must specify an input file" );
  fd = open( argv[1], O_RDONLY );
  if ( fd < 0 )
    nl_error( 3, "Error %d opening: %s", errno, strerror(errno) );
  for (;;) {
    nb = read( fd, buf, BUFSIZE );
    if ( nb < 0 )
      nl_error( 3, "Error %d on read: %s", errno, strerror(errno) );
    else if ( nb == 0 )
      nl_error( 3, "read returned zero bytes" );
    else {
      buf[nb] = '\0';
      nl_error( 0, "Read returned: %s", buf );
    }
  }
}

