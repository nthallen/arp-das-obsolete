/* readgc1.c
 * This program uses the tt_gc_chan() call to read each GC
 * channel 8 times per second. This is the function I intend
 * to use for TMC acquisition.
 */
#include <stdio.h>
#include <conio.h>
#include <i86.h>
#include "ttdriver.h"
void main( void ) {
  int i;

  tt_init();  
  tt_gc_reset();
  while ( !kbhit() ) {
	for ( i = 0; i < N_GC_CHANNELS; i++ ) {
	  printf( "  %10lu", tt_gc_chan( i ) );
	}
	putchar( '\n' );
	fflush( stdout );
	delay( 125 );
  }
  getch();
}
