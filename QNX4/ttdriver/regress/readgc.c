/* readgc.c
 * This program uses the tt_gc_read() call to get all the
 * GC counter data which is available once per second.
 * Actually, since the timing here is done crudely, the
 * period is somewhat longer than a second, so in steady
 * state you should occasionally see more than 8 samples
 * per cycle.
 */
#include <stdio.h>
#include <conio.h>
#include <i86.h>
#include "ttdriver.h"
void main( void ) {
  int i, j;

  tt_init();  
  tt_gc_reset();
  /* delay( 3000 ); */ /* too test overflow */
  while ( !kbhit() ) {
	fputs( "\033H", stdout );
	{ gc_data_buf gcbuf;
	  int offsets[N_GC_CHANNELS];

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
	delay( 1000 );
  }
}
