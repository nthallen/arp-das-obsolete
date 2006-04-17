#include <i86.h>

int main(void) {
  int i;
  
  for (i = 0; i < 5; i++) {
	sound(i&1 ? 494 : 440);
	delay(125);
  }
  sound(763);
  nosound();
  return(0);
}
