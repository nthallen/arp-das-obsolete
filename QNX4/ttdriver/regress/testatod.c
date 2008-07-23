/* testatod.c
 * This is a very rudimentary A/D diagnostic which continually
 * reads all the A/D channels.
 */
#include <stdio.h>
#include <conio.h>
#include "ttdriver.h"
void main( void ) {
  int i, j;

  tt_init();  
  putchar( '\f' );
  while ( ! kbhit() ) {
	fputs( "\033H", stdout );
	for ( i = 0; i < 96; i += 8 ) {
	  for ( j = 0; j < 8; j++ ) {
		printf( " %04X", tt_read_atod( i + j ) );
	  }
	  putchar( '\n' );
	}
  }
}
