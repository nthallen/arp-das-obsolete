/*
	Encoder Card Interface.
	Sends commands (RF) via encoder card.
	Ported to QNX 4.01 3/31/92 by Eil.
*/

#include <assert.h>
#include <conio.h>
#include <i86.h>
#include <sounds.h>

#define CMD_DISCRETE 0
#define CMD_DATA_LSB 0x40
#define CMD_DATA_MSB 0x80

void strobe(unsigned int address, unsigned int data, int mode) {
  outp(0x3E8, (unsigned char)(address | ((data & 3) << 6)));
  outp(0x3E9, (unsigned char)(((data >> 2) & 0x3F) | (mode & 0xC0)));
  outp(0x3EA, 2); /* Key Xmit */
  delay(150);     /* Let xmitter come to full power */
  outp(0x3EA, 3); /* Key Xmit + Larse */
  delay(10);
  outp(0x3EA, 2); /* Key Xmit - Larse */
  delay(150);     /* Make sure command is transmitted */
  outp(0x3EA, 0); /* Unkey Xmit */
}

void send_cmd(unsigned int address, unsigned int data, int nbytes) {

  assert(nbytes == 1 || nbytes == 2);
  if (nbytes == 1) strobe(address, data, CMD_DISCRETE);
  else {
    strobe(address, data & 0xFF, CMD_DATA_LSB);
    strobe(address, (data>>8) & 0xFF, CMD_DATA_MSB);

  }
  PASS_TUNE;
}

void init_8255(void) {
  /* Initialize 8255 */
  outp(0x3EB, 0x88);
  outp(0x3EA, 0);
}

/*int main(void) {
  unsigned int address, data;

  address = 027;
  for (data = 0;; data++) {
    if (kbhit()) break;
    send_cmd(address, data, 2);
    printf("Address: %o   Data %04X\n", address, data);
  }
  while (kbhit()) getch();
}
*/
