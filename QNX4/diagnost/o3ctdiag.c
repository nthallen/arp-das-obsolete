/* irqtest.c
 * $Log$
 * Copied from ozone.col
 * $Id$
 */
#include <stdio.h>
#include <sys/dev.h>
#include <sys/types.h>
#include <sys/proxy.h>
#include <sys/irqinfo.h>
#include <sys/kernel.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <conio.h>
#include "subbus.h"
#include "diaglib.h"

#ifdef __USAGE
%C [IRQ n] [EIR m] [HEX] [MUL p]
   where n is between 2 and 15 (default 9)
   and m is between 0 and 15
   HEX specifies hex output (default is decimal)
   MUL specifies a multiplication factor p
#endif

static pid_t onint_proxy, kbd_proxy;
static int onint_iid;
#define IRQ_NUM 9
#define FULL_COUNTS
  
static pid_t far onint_handler(void) { return(onint_proxy); }

#ifdef FULL_COUNTS
#define OUT_DEC 0
#define OUT_HEX 1
static int outfmt = OUT_DEC;
static int mulfact = 1;

static unsigned char data[10];

void read_counters(unsigned char *dbuf) {
  (*((unsigned int *) (&dbuf[0]))) = read_subbus(0, 0x680);
  dbuf[2] = read_subbus(0, 0x682) & 0xFF;
  (*((unsigned int *) (&dbuf[3]))) = read_subbus(0, 0x684);
  dbuf[5] = read_subbus(0, 0x686) & 0xFF;
  (*((unsigned int *) (&dbuf[6]))) = read_subbus(0, 0x688);
  (*((unsigned int *) (&dbuf[8]))) = read_subbus(0, 0x68A);
}

void display_counters(unsigned char *dbuf) {
  int i;
  unsigned long int la;

  if (outfmt == OUT_HEX) {
    for (i = 2; i >= 0; i--) printf("%02X", (unsigned int) dbuf[i]);
    printf("    ");
    for (i = 5; i >= 3; i--) printf("%02X", (unsigned int) dbuf[i]);
    printf("    ");
    for (i = 7; i >= 6; i--) printf("%02X", (unsigned int) dbuf[i]);
    printf("    ");
    for (i = 9; i >= 8; i--) printf("%02X", (unsigned int) dbuf[i]);
  } else {
    la = (unsigned long int) (*(unsigned int *)(&dbuf[0]));
    la += dbuf[2] * 0x10000L;
    la *= mulfact;
    printf("%8lu    ", la);
    la = (unsigned long int) (*(unsigned int *)(&dbuf[3]));
    la += dbuf[5] * 0x10000L;
    la *= mulfact;
    printf("%8lu    ", la);
    la = (unsigned long int) (*(unsigned int *)(&dbuf[6]));
    la *= mulfact;
    printf("%6lu    ", la);
    la = (unsigned long int) (*(unsigned int *)(&dbuf[8]));
    la *= mulfact;
    printf("%6lu", la);
  }
  putchar('\n');
}
#endif
  
int main(int argc, char **argv) {
  int i, irq = IRQ_NUM, eir = -1, val;
  pid_t who;
  unsigned old_mode;

  printf("\nOzone Board (Rev. B,QNX) Counter Diagnostic " __DATE__ "\n");
  for (i = 1; i < argc; i++) {
    if (stricmp(argv[i], "IRQ") == 0) {
      if (++i < argc && sscanf(argv[i], "%d", &val) == 1 &&
          val >= 2 && val <= 15)
        irq = val;
    } else if (stricmp(argv[i], "EIR") == 0) {
      if (++i < argc && sscanf(argv[i], "%d", &val) == 1 &&
          val >= 0 && val <= 15)
        eir = val;
    } else if (stricmp(argv[i], "HEX") == 0) outfmt = OUT_HEX;
    else if (stricmp(argv[i], "MUL") == 0) {
      if (++i < argc && sscanf(argv[i], "%d", &val) == 1 &&
          val >= 0 && val <= 8)
        mulfact = val;
    }
  }
  
  if (load_subbus() == 0) diag_error("No subbus\n");

  /* Attach Proxies */
  onint_proxy = qnx_proxy_attach(0, NULL, 0, -1);
  if (onint_proxy == -1)
	diag_error("Error attaching onint proxy: %d - %s\n",
				errno, strerror(errno));
  kbd_proxy = qnx_proxy_attach(0, NULL, 0, -1);
  if (kbd_proxy == -1)
	diag_error("Error attaching kb proxy: %d - %s\n",
				errno, strerror(errno));

  /* Attach the interrupt */
  onint_iid = qnx_hint_attach(irq, onint_handler, FP_SEG(&onint_proxy));
  if (onint_iid == -1)
	diag_error("Error attaching interrupt for onint: %d - %s\n",
				errno, strerror(errno));

  /* Arm the keyboard */
  old_mode = dev_mode(STDIN_FILENO, 0, _DEV_EDIT | _DEV_ECHO);
  if (dev_arm(STDIN_FILENO, kbd_proxy, _DEV_EVENT_INPUT))
	diag_error("Error arming stdin: %d - %s\n",
				errno, strerror(errno));

  read_subbus(0, 0x680);
  printf("Waiting for IRQ %d\n", irq);
  fflush(stdout);
  for (i = 0;;) {
	who = Receive(0, NULL, 0);
	if (who == onint_proxy) {
	  #ifdef FULL_COUNTS
		read_counters(&data);
		display_counters(&data);
	  #else
		printf("Interrupt %d\n", ++i);
		fflush(stdout);
		read_subbus(0, 0x680);
	  #endif
	} else break;
  }
  qnx_hint_detach(onint_iid);
  qnx_proxy_detach(onint_proxy);
  qnx_proxy_detach(kbd_proxy);
  while (kbhit()) getch();
  dev_mode(STDIN_FILENO, old_mode, _DEV_MODES);
  if (who != kbd_proxy) diag_error("Received from someone else!\n");
  return(0);
}
