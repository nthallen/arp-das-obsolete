/* benchatod.c
 * The purpose of this little program is simply to try to 
 * determine how long an A/D read through ttdriver takes
 */
#include "ttdriver.h"

#define LOOP 10000
void main( void ) {
  int i;
  for ( i = 0; i < LOOP; i++ )
    tt_read_atod( 0 );
}
