#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "nortlib.h"
#include "tm.h"

int main( int argc, char **argv ) {
  if ( argc < 2 ) TM_fd = STDIN_FILENO;
  else {
    TM_fd = open( argv[1], O_RDONLY );
    if ( TM_fd < 0 ) nl_error( 3, "Error opening '%s'", argv[1] );
  }
  TM_readfd();
  printf( "Done\n" );
  return 0;
}

void TM_init( void ) {
  printf( "Init\n" );
}

void TM_tstamp( int tstype, mfc_t mfctr, time_t time ) {
  printf( "Timestamp: %d %ld\n", mfctr, time );
}

void TM_row( mfc_t mfctr, int row, const unsigned char *data ) {
  printf( "MFCtr = %5d\n", mfctr );
}

void TM_quit( void ) {
  printf( "Quit\n" );
}

