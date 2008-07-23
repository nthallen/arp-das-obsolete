#include "subbus.h"
char rcsid_subbus_sbba_c[] =
  "$Header$";

unsigned int sbba(unsigned int addr) {
  unsigned int word;
  
  if ( read_ack( 0, addr, &word ) ) {
	if (addr & 1) word >>= 8;
	return( word & 0xFF );
  } else return 0;
}

unsigned int sbwa(unsigned int addr) {
  unsigned int word;
  
  if ( read_ack( 0, addr, &word ) )
	return word;
  else return 0;
}
