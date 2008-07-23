/* read_atod( x )
   CFG  <- TT_READ_CFG      A in, Ch in, Cl out, B out
   CTRL <- 1         STRB/ High
   ADDR <- channel
   CTRL <- 0         STRB/ Low: begin acquisition & conversion
   read CTRL while EOC != 0 ( bit 0x10 )
   lsb <- DATA
   CTRL <- 2
   MSB <- DATA
   ADDR <- dummy_address
*/
#include <conio.h>
#include "ttdriver.h"
#include "ttintern.h"
unsigned short tti_read_atod( unsigned short channel ) {
  unsigned short val;
  static short retry = 0;
  
  if ( channel > 95 ) return ~0;
  if ( retry > 0 ) {
	retry--;
	return ~0;
  }
  outp( TT_CFG_PORT, TT_READ_CFG ); /* A in, Ch in, Cl out, B out */
  outp( TT_CTRL_PORT, 1 );   /* STRB/ High */
  outp( TT_ADDR_PORT, channel ); /* output the address */
  outp( TT_CTRL_PORT, 0 );   /* STRB/ Low to start acquisition & conversion */
  start_timer();
  while ( !timed_out && ( inp( TT_CTRL_PORT ) & 0x10 ) != 0 ) ;
  stop_timer();
  if ( timed_out ) {
	retry = 1000;
	val = ~0;
  } else {
	val = inp( TT_DATA_PORT ) & 0xFF;
	outp( TT_CTRL_PORT, 2 );   /* Set up for MSNybble */
	val += ( inp( TT_DATA_PORT ) & 0xF ) << 8;
  }
  outp( TT_ADDR_PORT, TT_DUMMY_ADDR );
  return val;
}
