/* digital2.c is a diagnostic for checking out any and all digital
 * input or output boards in their flight configurations.
 * $Log$
 * Revision 1.2  1992/07/23  19:14:46  nort
 * Mods for QNX4
 *
 * Revision 1.1  1992/07/22  14:24:15  nort
 * Initial revision
 *
 * Modified from digital to incorporate everything. Dec. 23, 1991
 * Modified from digio2.c July 22, 1990
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __QNX__
  #include <conio.h>
#else
  #include <dos.h>
  #include "reslib.h"
#endif
#include "subbus.h"
static char rcsid[] = "$Id$";

/* The test will not require the user to select which board s/he wishes
   to test.  The diagnostic knows which ports are input and which
   are output.  All output ports are initialized to output high
   levels.  When an output port is selected, the program will toggle
   individual data lines, allowing the tester to check the output
   with an oscilloscope or volt meter.

   The test will first attempt to address every board. If the board
   acknowledges, it will be presumed present (unless permanent ack
   is detected). On all present boards, patterns will be written
   to (all, some) output ports and read back. Any problems will
   be reported.
  
   PgDn		Advance to the next port
   PgUp		Return to the previous port
   Right Arrow	Advance to the next bit (ignored on input ports)
   Left Arrow	Return to the previous bit (ignored on input ports)
   Space	Single step toggles
   Enter	Toggle at will (default)
   Esc		Exit diagnostic
   C		Toggle cmdenbl/ (asserted by default)
   S		Toggle cmdstrobe
*/
#ifdef __QNX__
  #define EXTENDED_KEY 0xFF
  #define	KEY_PGUP	0x01A2
  #define	KEY_PGDN	0x01AA
  #define	KEY_LEFT	0x01A4
  #define	KEY_RIGHT	0x01A6
  #define	KEY_DOWN	0x01A9
  #define	KEY_UP		0x01A1
  #define	KEY_F1		0x0181
#else
  #define EXTENDED_KEY 0
  #define	KEY_PGUP	0x0149
  #define	KEY_PGDN	0x0151
  #define	KEY_LEFT	0x014b
  #define	KEY_RIGHT	0x014d
  #define	KEY_DOWN	0x0150
  #define	KEY_UP		0x0148
  #define	KEY_F1		0x013b
#endif

char *help_text[] = {
  "   PgDn\t\tAdvance to the next port",
  "   PgUp\t\tReturn to the previous port",
  "   Right Arrow\tAdvance to the next bit (ignored on input ports)",
  "   Left Arrow\tReturn to the previous bit (ignored on input ports)",
  "   Space\tSingle step toggles",
  "   Enter\tToggle at will (default)",
  "   Esc\t\tExit diagnostic",
  "   C\t\tToggle cmdenbl/ (asserted by default)",
  "   S\t\tToggle cmdstrobe",
  "   H or F1\tDisplay this message",
  NULL
};

void help(void) {
  char **p;

  putchar('\n');
  for (p = help_text; *p != NULL; p++) printf("%s\n", *p);
}

/* Terminates with an error message. Could be expanded to take
   additional arguments if necessary.
*/
void app_error(char *text) {
  fprintf(stderr, "%s\n", text);
  exit(1);
}

/* The following information is needed for each port:
    Input or Output?
    Are the connector pins in order? (FL_SHUFFLED)
    Address (high or low byte is implicit)
    Which chip and port
    Which connector and pins
    What order are the bits on the buffer?
*/
#define FL_OUTPUT 1
#define FL_INPUT 0
#define FL_SHUFFLED 2

struct bddf {
  int present;
  char *name;
  unsigned int control;
  unsigned int initcmd;
} boards[] = {
  0, "Disc0", 0x806, 0x8989,
  0, "Disc1", 0x826, 0x8989,
  0, "Disc2", 0x846, 0x8989,
  0, "Dstat", 0x406, 0x9B9B,
  0, "Digio0 Chips 1&3", 0x866, 0x8282,
  0, "Digio0 Chips 2&4", 0x886, 0x8989,
  0, "Digio1 Chips 1&3", 0x8A6, 0x8282,
  0, "Digio1 Chips 2&4", 0x8C6, 0x8989,
  0, "Main Ozone", 0x8E6, 0x0089
};
#define N_BOARDS (sizeof(boards)/sizeof(struct bddf))

unsigned int pattern[] = { 0, 0x5555, 0xAAAA, 0xFFFF, 0x0101, 0x0202,
  0x0404, 0x0808, 0x1010, 0x2020, 0x4040, 0x8080 };
#define N_PATS (sizeof(pattern)/sizeof(unsigned int))

/* orders defines the order of bits on the buffer chips */
int orders[5][8] = {
  9,7,5,3,2,4,6,8,
  7,8,9,6,5,4,3,2,
  9,8,7,6,5,4,3,2,
  3,4,5,6,7,8,9,2,
  2,3,4,5,6,7,8,9
};

struct pd {
  int board;
  char *chip;
  char port;
  unsigned int address;
  char *bufchip;
  int bufpinorder;
  char *con_name;
  int con_pin;
  int flags;
} port_data[] = {
  0, "Z9", 'A', 0x808, "Z11", 0, "J8", 2, FL_OUTPUT,
  0, "Z9", 'B', 0x80A, "Z12", 1, "J8", 10, FL_OUTPUT,
  0, "Z10", 'A', 0x811, "Z14", 0, "J8", 21, FL_OUTPUT,
  0, "Z10", 'B', 0x813, "Z13", 2, "J8", 29, FL_OUTPUT,
  1, "Z9", 'A', 0x828, "Z11", 0, "J8", 2, FL_OUTPUT,
  1, "Z9", 'B', 0x82A, "Z12", 1, "J8", 10, FL_OUTPUT,
  1, "Z10", 'A', 0x831, "Z14", 0, "J8", 21, FL_OUTPUT,
  1, "Z10", 'B', 0x833, "Z13", 2, "J8", 29, FL_OUTPUT,
  2, "Z9", 'A', 0x848, "Z11", 0, "J8", 2, FL_OUTPUT,
  2, "Z9", 'B', 0x84A, "Z12", 1, "J8", 10, FL_OUTPUT,
  2, "Z10", 'A', 0x851, "Z14", 0, "J8", 21, FL_OUTPUT,
  2, "Z10", 'B', 0x853, "Z13", 2, "J8", 29, FL_OUTPUT,
  3, "Z9", 'A', 0x408, "Z11", 0, "J8", 2, FL_INPUT,
  3, "Z9", 'B', 0x40A, "Z12", 1, "J8", 10, FL_INPUT,
  3, "Z10", 'A', 0x411, "Z14", 0, "J8", 21, FL_INPUT,
  3, "Z10", 'B', 0x413, "Z13", 2, "J8", 29, FL_INPUT,
  4, "U1", 'A', 0x868, "U11", 2, "J8",  2, FL_OUTPUT,
  4, "U1", 'B', 0x86A, "U12", 2, "J8", 10, FL_INPUT,
  4, "U1", 'C', 0x86C, "U13", 3, "J8", 18, FL_OUTPUT | FL_SHUFFLED,
  5, "U2", 'A', 0x888, "U15", 2, "J8", 26, FL_OUTPUT,
  5, "U2", 'B', 0x88A, "U16", 2, "J8", 34, FL_OUTPUT,
  5, "U2", 'C', 0x88C, "U17", 3, "J8", 42, FL_INPUT | FL_SHUFFLED,
  4, "U3", 'A', 0x871, "U19", 2, "J9",  2, FL_OUTPUT,
  4, "U3", 'B', 0x873, "U20", 2, "J9", 10, FL_INPUT,
  4, "U3", 'C', 0x875, "U21", 3, "J9", 18, FL_OUTPUT | FL_SHUFFLED,
  5, "U4", 'A', 0x891, "U23", 2, "J9", 26, FL_OUTPUT,
  5, "U4", 'B', 0x893, "U24", 2, "J9", 34, FL_OUTPUT,
  5, "U4", 'C', 0x895, "U25", 3, "J9", 42, FL_INPUT | FL_SHUFFLED,
  6, "U1", 'A', 0x8A8, "U11", 2, "J8",  2, FL_OUTPUT,
  6, "U1", 'B', 0x8AA, "U12", 2, "J8", 10, FL_INPUT,
  6, "U1", 'C', 0x8AC, "U13", 3, "J8", 18, FL_OUTPUT | FL_SHUFFLED,
  7, "U2", 'A', 0x8C8, "U15", 2, "J8", 26, FL_OUTPUT,
  7, "U2", 'B', 0x8CA, "U16", 2, "J8", 34, FL_OUTPUT,
  7, "U2", 'C', 0x8CC, "U17", 3, "J8", 42, FL_INPUT | FL_SHUFFLED,
  6, "U3", 'A', 0x8B1, "U19", 2, "J9",  2, FL_OUTPUT,
  6, "U3", 'B', 0x8B3, "U20", 2, "J9", 10, FL_INPUT,
  6, "U3", 'C', 0x8B5, "U21", 3, "J9", 18, FL_OUTPUT | FL_SHUFFLED,
  7, "U4", 'A', 0x8D1, "U23", 2, "J9", 26, FL_OUTPUT,
  7, "U4", 'B', 0x8D3, "U24", 2, "J9", 34, FL_OUTPUT,
  7, "U4", 'C', 0x8D5, "U25", 3, "J9", 42, FL_INPUT | FL_SHUFFLED,
  8, "U20", 'A', 0x8E0, "U23", 4, "VC2", 2, FL_OUTPUT,
  8, "U20", 'B', 0x8E2, "U24", 4, "VC2", 18, FL_OUTPUT,
  8, "U20", 'C', 0x8E4, "U25", 4, "VC2", 10, FL_INPUT
};
#define N_PORTS (sizeof(port_data)/sizeof(struct pd))

/* state definitions for the state variable */
#define STATE_EXIT 0
#define STATE_NEW_PORT 1
#define STATE_NEW_BIT 2
#define STATE_LOOPING 3
/* state definitions for the looping variable */
#define LOOP_WILDLY 0
#define LOOP_ONCE 1
#define LOOP_HOLD 2

int kgetch(void) {
  int c;

  if (kbhit()) {
    c = getch();
    if (c == EXTENDED_KEY) c = 0x100 | getch();
  } else c = -1;
  return c;
}

void set_cmdstrobe(int asserted) {
  if (asserted) outp(0x30E, 3);
  else outp(0x30E, 2);
}

/* Z9-4|----|Z11-9  Z11-11|-------|J8-2 */
void identify_pin(int port, int bit) {
  struct pd *p_d;
  int ppipin, bpin, cpin;

  p_d = &port_data[port];
  if (p_d->port == 'A') {
    if (bit < 4) ppipin = 4 - bit;
    else ppipin = 44 - bit;
  } else ppipin = bit + 18;
  printf("%s-P%c%d pin %d <----> ", p_d->chip, p_d->port, bit, ppipin);
  bpin = orders[p_d->bufpinorder][bit];
  printf("%s-%d and %s->%d <----> ", p_d->bufchip, bpin, p_d->bufchip, 20-bpin);
  cpin = p_d->con_pin +
    ((p_d->flags & FL_SHUFFLED) ? (7 - ((bit+1) & 7)) : bit);
  printf("%s-%d\n", p_d->con_name, cpin);
}

int new_port(int port, int direction) {
  for (;;) {
    if (direction == 0) direction = 1;
    else if (direction == 1) {
      if (++port == N_PORTS) port = 0;
    } else if (port == 0) port = N_PORTS-1;
    else port--;
    if (boards[port_data[port].board].present) break;
  }
  return(port);
}

void main(void) {
  int port, bit, shift, pin, i, j;
  int state, looping, c;
  unsigned int value, mask, addr;
  int cmdenbl = 1, cmdstrobe = 0;
  struct pd *p_d;

  printf("Digital Command and Status Diagnostic: " __DATE__ "\n");
  pin = load_subbus();
  if (pin == 0) app_error("Subbus library not resident");
  if (pin != SB_SYSCON)
    printf("CMDSTROBE testing will not work properly "
           "without the system controller\n");

  /* Change this to search for boards and then test them */
  for (i = 0; i < N_BOARDS; i++)
    if (write_ack(0, boards[i].control, boards[i].initcmd) != 0) {
      boards[i].present = 1;
      printf("%s present\n", boards[i].name);
    }
  for (i = 0, port = 0; port < N_PORTS; port++) {
    if (boards[port_data[port].board].present) {
      i++;
      if (port_data[port].flags & FL_OUTPUT) {
	addr = port_data[port].address;
	mask = (addr & 1) ? 0xFF00 : 0xFF;
	value = 0;
	for (j = 0; j < N_PATS; j++) {
	  write_subbus(0, addr, pattern[j]);
          write_subbus(0, 0, (unsigned int)(~pattern[j]));
	  value |= (read_subbus(0, addr) ^ pattern[j]) & mask;
	}
	write_subbus(0, addr, 0);
	if (value != 0)
	  printf("  Readback error: %s %s port %c. Bits failing: %04X\n",
	      boards[port_data[port].board].name,
	      port_data[port].chip, port_data[port].port, value);
      }
    }
  }
  if (i == 0) app_error("No boards present");
  set_cmdenbl(cmdenbl);
  set_cmdstrobe(cmdstrobe);
  help();
  looping = LOOP_WILDLY;
  port = new_port(0, 0);
  for (state = STATE_NEW_PORT; state == STATE_NEW_PORT;) {
    /* set up for toggling new port/bit */
    p_d = &port_data[port];
    shift = (p_d->address & 1) ? 8 : 0;
    if (p_d->flags & FL_OUTPUT) {
      bit = 0;
      printf("\nWriting to ");
    } else {
      value = 0x100;
      printf("\nReading from ");
    }
    printf("%s 8255 %s port %c on connector pins %d-%d\n",
	   boards[p_d->board].name, p_d->chip, p_d->port,
	   p_d->con_pin, p_d->con_pin+7);

    /* Loop for selecting bits */
    for (state = STATE_NEW_BIT; state == STATE_NEW_BIT; ) {
      if (p_d->flags & FL_OUTPUT) {
        value = 0;
	mask = 1 << (shift+bit);
	printf("Toggling ");
	identify_pin(port, bit);
	if (looping == LOOP_HOLD) looping = LOOP_ONCE;
      } else {
	printf(" Bits 0-7 are located on the following pins:\n");
	for (bit = 0; bit < 8; bit++) identify_pin(port, bit);
      }

      /* Here's the tight loop for toggling bits */
      for (state = STATE_LOOPING; state == STATE_LOOPING;) {

	/* first, we'd better tick the SIC periodically */
	tick_sic();

	/* perform the tight loop action */
	/* If output, flip the bit; if input, display the port */
	if ((p_d->flags & FL_OUTPUT) == 0) {
	  mask = (read_subbus(0, p_d->address) >> shift) & 0xFF;
	  if (mask != value) {
	    value = mask;
	    for (mask = 0x80; mask != 0; mask >>= 1)
	      putchar((value & mask) ? '1' : '0');
	    putchar('\r');
		fflush(stdout);
	  }
	} else if (looping == LOOP_WILDLY || looping == LOOP_ONCE) {
	  value ^= mask;
	  write_subbus(0, p_d->address, value);
	  if (looping == LOOP_ONCE) {
	    printf((value & mask) ? "HIGH     \r" : "LOW      \r");
		fflush(stdout);
	    looping = LOOP_HOLD;
	  }
	}

	/* Then poll for keyboard input */
	switch (c = kgetch()) {
	  case -1:
	    break;
	  case ' ': /* Single step toggles */
	    looping = LOOP_ONCE;
	    break;
	  case '\r': /* Toggle at will (default) */
	  case '\n':
	    if (p_d->flags & FL_OUTPUT) {
	      printf("TOGGLING\r");
		  fflush(stdout);
		}
	    looping = LOOP_WILDLY;
	    break;
	  case '\033': /* Exit diagnostic */
	    state = STATE_EXIT;
	    break;
	  case 'C': /* Toggle cmdenbl/ (asserted by default) */
	  case 'c':
	    cmdenbl ^= 1;
	    set_cmdenbl(cmdenbl);
  	    if (cmdenbl) printf("\nCMDENBL is asserted\n");
	    else printf("\nCMDENBL is not asserted\n");
	    break;
	  case 'S': /* Toggle cmdstrobe */
	  case 's':
	    cmdstrobe ^= 1;
	    set_cmdstrobe(cmdstrobe);
  	    if (cmdstrobe) printf("\nCMDSTROBE is asserted\n");
	    else printf("\nCMDSTROBE is not asserted\n");
	    break;
	  case KEY_PGUP: /* Return to the previous port */
	    if (p_d->flags & FL_OUTPUT)
	      write_subbus(0, p_d->address, 0);
	    port = new_port(port, -1);
	    state = STATE_NEW_PORT;
	    break;
	  case KEY_PGDN: /* Advance to the next port */
	    if (p_d->flags & FL_OUTPUT)
	      write_subbus(0, p_d->address, 0);
	    port = new_port(port, 1);
	    state = STATE_NEW_PORT;
	    break;
	  case KEY_LEFT: /* Return to the previous bit */
	  case KEY_DOWN: /* (ignored on input ports) */
	    if (p_d->flags & FL_OUTPUT) {
	      if (bit == 0) bit = 7;
	      else bit--;
	      state = STATE_NEW_BIT;
	    }
	    break;
	  case KEY_RIGHT: /* Advance to the next bit */
	  case KEY_UP: /* (ignored on input ports) */
	    if (p_d->flags & FL_OUTPUT) {
	      if (bit == 7) bit = 0;
	      else bit++;
	      state = STATE_NEW_BIT;
	    }
	    break;
	  case KEY_F1:
	  case 'H':
	  case 'h':
	    help();
	    break;
	}
      }
    }
  }
  disarm_sic();
  putc('\n', stdout);
}
