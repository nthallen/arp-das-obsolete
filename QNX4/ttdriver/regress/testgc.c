#include <stdio.h>
#include <conio.h>
#include <i86.h>
#include "ttdriver.h"
#define TESTINGGC
void main( void ) {
  int i, j;

  tt_init();  
  tt_gc_reset();
  /* delay( 1000 ); */
  for ( j = 0; j < 100; j++ ) {
	for ( i = 0; i < 10; i++ ) {
	  int byte;
	  byte = tt_gc_byte();
	  printf( "%5d", byte );
	}
	putchar( '\n' );
  }
}
	#ifdef GC_READ_TEST
	fputs( "\033H", stdout );
	delay( 1000 );
	{ gc_data_buf gcbuf;
	  int offsets[N_GC_CHANNELS], samples[N_GC_CHANNELS];

	  if ( tt_gc_read( &gcbuf ) == 0 ) {
		for ( i = 0; i < N_GC_CHANNELS; i++ ) {
		  offsets[i] = (i > 0) ? gcbuf.end_offset[i-1] : 0;
		}
		for ( j = 0; j < MAX_GC_SAMPLES; j++ ) {
		  for ( i = 0; i < N_GC_CHANNELS; i++ ) {
			if (offsets[i] < gcbuf.end_offset[i])
			  printf( "  %8lu", gcbuf.samples[ offsets[i]++ ] );
			else printf( "          " );
		  }
		  putchar( '\n' );
		}
	  } else {
		printf( "tt_gc_read returned an error\n" );
	  }
	}
	#endif
