/* sbb.c */
#include "subbus.h"
char rcsid_subbus_sbb_c[] =
  "$Header$";

unsigned int sbb(unsigned int addr) {
  unsigned int word;
  
  word = read_subbus(0, addr);
  if (addr & 1) word >>= 8;
  return(word & 0xFF);
}
