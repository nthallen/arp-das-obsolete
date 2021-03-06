/* dtoatest.c
 * $Log$
 * Revision 1.1  1992/08/03  15:24:52  nort
 * Initial revision
 *
   A test to put a dtoa circuit through some hoops.
   Specify the address and the step size (in hex).
      dtoatest address stepsize [stepwise] [start n] [min n] [max n]
   The program will continuously ramp the data until escape is hit.
   In the stepwise mode, a character must be hit at each step.
   If a backspace is hit in stepwise mode, a step is taken in the
   opposite direction.  In continuous mode
   
	C		go into continuous mode
	S		Change the stepsize
	V		Change the output value
	m		Change minimum output limit
	M		Change maximum output limit
	Enter		Single step forward
	Backspace	Single step backward
	Esc		Exit
*/
static char rcsid[] = "$Id$";

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#ifdef __QNX__
  #include <conio.h>
  #define stcl_h(buf,val) strlen(ultoa(val, buf, 16))
#else
  #include <dos.h>
  #include "reslib.h"
  #include "adu.h"
#endif
#include "subbus.h"
#include "diaglib.h"

void app_error(char *fmt, ...) {
  va_list arg;
  
  va_start(arg, fmt);
  vfprintf(stderr, fmt, arg);
  va_end(arg);
  fputc('\n', stderr);
  exit(1);
}

long int stch_sl(char *str) {
  int negative;
  long int li;

  if (*str == '-') {
    negative = 1;
    str++;
  } else negative = 0;
  li = strtol(str, NULL, 16);
  if (negative) li = -li;
  return(li);
}

void newval(char *prompt, unsigned int *value) {
  char buf[80];

  printf("\n%s: ", prompt);
  gets(buf);
  *value = (int) strtol(buf, NULL, 16);
}

char help_text[] =
	"\tH\t\tDisplay this message\n"
	"\tC\t\tGo into continuous mode\n"
	"\tS\t\tChange the stepsize\n"
	"\tV\t\tChange the output value\n"
	"\tm\t\tChange minimum limit value\n"
	"\tM\t\tChange maximum limit value\n"
	"\tEnter\t\tSingle step forward\n"
	"\tBackspace\tReverse direction\n"
	"\tEsc\t\tExit\n\n";

void main(int argc, char **argv) {
  int stepwise = 0, ad664 = 0, i, c, wack, rack;
  unsigned int address, minv = 0, maxv = 0xFFFF, tmpo;
  long int stepsize = 1;
  long int output = 0L;
  char buf[48];

  if (argc < 2) {
    printf("dtoatest address [options]\n");
    printf("  where [options] are any of the following:\n");
    printf("  stepsize n\n"
	   "  stepwise\n"
	   "  start n\n"
	   "  min n\n"
	   "  max n\n"
	   "  ad664\n"
	   );
    exit(1);
  }
  address = (int) strtol(argv[1], NULL, 16);
  for (i = 2; i < argc; i++) {
    if (stricmp(argv[i], "stepwise") == 0) stepwise = 1;
    else if (strnicmp(argv[i], "sta", 3) == 0)
      output = strtol(argv[++i], NULL, 16);
    else if (strnicmp(argv[i], "steps", 5) == 0) {
      stepsize = stch_sl(argv[++i]);
      if (stepsize == 0) stepsize = 1;
    } else if (stricmp(argv[i], "min") == 0)
	  minv = (int) strtol(argv[++i], NULL, 16);
    else if (stricmp(argv[i], "max") == 0)
	  maxv = (int) strtol(argv[++i], NULL, 16);
    else if (stricmp(argv[i], "ad664") == 0) ad664 = 1;
    else app_error("Unrecognized argument: \"%s\"", argv[i]);
  }
  printf("DtoAtest workout program, " __DATE__ "\n");
  if (load_subbus() == 0) app_error("Resident subbus library required");
  
  if (ad664) {
	check_ack(address, 0);
	check_rw(address, 0xFFF);
  } else check_wack(address, 0);
  
  printf("%s", help_text);

  if (ad664) maxv &= 0xFFF;	/* Ozone board is only 12 bits */
  for (;;) {
    if (output < minv) output = maxv;
    else if (output > maxv) output = minv;
    wack = write_ack(0, address, (unsigned int)output);
    i = stcl_h(buf, output);
    while (i++ < 4) putchar('0');
    fputs(buf, stdout);
    putchar(wack ? ' ' : '!');
    if (ad664) {	/* Check read-back capability */
      write_subbus(0,0,(unsigned int)(~output)); /* helps see bad data */
      if (rack = read_ack(0, address, &tmpo)) {
        tmpo &= 0xFFF;
        if (tmpo != (((unsigned int)output) & 0xFFF)) {
          printf("  Read %04x", tmpo);
          rack = 0;
		}
      } else printf("  No Ack on Read");
    } else rack = 1;
    putchar(wack && rack ? '\r' : '\n');
	fflush(stdout);
    if (kbhit() || stepwise) {
      c = getch();
      if (c == 0) c = 0x100 | getch();
      switch (c) {
	case 0x013b:	/* F1 */
	case 'H':
	case 'h':
	  printf("\n%s\nHit any key to continue:", help_text);
	  getch();
	  putchar('\n');
	  break;
	case 'C':
	case 'c':	/* go into continuous mode */
	  stepwise = 0;
	  break;
	case '\n':
	case '\r':	/* Single step forward */
	  stepwise = 1;
	  output += stepsize;
	  break;
	case '\b':
	case 127:
	  if (stepwise) output -= stepsize;
	  else stepsize = -stepsize;
	  break;
	case 'S':
	case 's':	/* Change the stepsize */
	  printf("\nNew step size: ");
	  gets(buf);
	  stepsize = stch_sl(buf);
	  if (stepsize == 0) stepsize = 1;
	  break;
	case 'V':
	case 'v':	/* Change the stepsize */
	  newval("New output value", &tmpo);
	  output = tmpo;
	  break;
	case 'm':
	  newval("New minimum limit value", &minv);
	  break;
	case 'M':
	  newval("New maximum limit value", &maxv);
	  break;
	case '\033':	/* Exit */
	case EOF:
	  exit(0);
      }
    } else output += stepsize;
  }
}
