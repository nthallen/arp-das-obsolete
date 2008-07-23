/* indxdiag.c Stepper Motor Indexer Diagnostic.
*/
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <conio.h>
#include "subbus.h"
#include "diaglib.h"

#ifdef SCRAMBLED
  unsigned short s_tab[8][2] = {
    0x100,  0x8000,
    0x200,  0x4000,
    0x400,  0x2000,
    0x800,  0x1000,
    0x1000, 0x400,
    0x2000, 0x800,
    0x4000, 0x200,
    0x8000, 0x100
  };

  unsigned short scramble(unsigned short data, int out) {
    unsigned short sdata;
    int from, to, i;

    from = out ? 1 : 0;
    to = 1 - from;
    sdata = data & 0xFF;
    for (i = 0; i < 8; i++)
      if (data & s_tab[i][from]) sdata |= s_tab[i][to];
    return(sdata);
  }

  #define WRACK(x,y) write_ack(0,x,scramble(y,1))
  #define WRSUB(x,y) write_subbus(0,x,scramble(y,1))
  #define RDSUB(x) scramble(read_subbus(0,x),0)
#else
  #define WRACK(x,y) write_ack(0,x,y)
  #define WRSUB(x,y) write_subbus(0,x,y)
  #define RDSUB(x) read_subbus(0,x)
#endif


unsigned int kgetch(void) {
  unsigned int r;
  
  r = getch();
  if (r == 0xFF) r = 0xFF00 | getch();
  return(r);
}
#define KG_ESCAPE '\033'
#define KG_F1 0xFF81
#define KG_F2 0xFF82
#define KG_F3 0xFF83
#define KG_F4 0xFF84
#define KG_F5 0xFF85
#define KG_F6 0xFF86
#define KG_F7 0xFF87
#define KG_F8 0xFF88
#define KG_F9 0xFF89
#define KG_F10 0xFF8A


void strzcpy(char *to, char *from) {
  while (*from != '\0') *to++ = *from++;
}

#define TITLE "Stepper Motor Indexer Diagnostic, " __DATE__
#define BASE_ADDR 0xA00
#define CHAN_BASE(x) (BASE_ADDR+8*(x))
#define HYST_ADDR CHAN_BASE(3)
#define STATUS(x) CHAN_BASE(x)
#define DRIVE_OUT(x) CHAN_BASE(x)
#define DRIVE_IN(x) (CHAN_BASE(x)+4)
#define DRIVE_INH(x) (CHAN_BASE(x)+6)
#define UP_DOWN(x) (CHAN_BASE(x)+2)

unsigned short display_status;
#define DST_INITIALIZE 1
#define DST_OUTPUT 2

/* Data structures defining the board data */
typedef struct {
  unsigned short position;
  unsigned char hw_status;
} ixch;
#define N_CHANNELS 3
#define HW_INL 1
#define HW_OUTL 2
#define HW_DIR 4
#define HW_DONE 8
#define HW_INK 0x10
#define HW_OUTK 0x20
#define HW_ZERO 0x40
#define update_bits(x,y,z) {\
	dspd.channels[x].hw_status=\
	  (dspd.channels[x].hw_status & ~(y)) | (z & (y));\
	display_status |= DST_OUTPUT; }
#define CHECK_BIT(x,f,y,z) if ((display_status & DST_INITIALIZE) || (diff & x)) {\
	  strzcpy(obuf+f, (value & x) ? y : z);\
	  update_bits(ch, x, value);\
	}

struct {
  ixch channels[N_CHANNELS];
  unsigned short hyst_value;
} crnt, dspd;

#define GST_HYST_ON 1
unsigned short global_status = 0;

/* Data structures defining the display */
#define CH_DISP "³xxxxx Off ÄÙ ÄÙ Ä ÀÄ ÀÄ"
#define CH_DISP "³xxxxx Off ÄÙ ÄÙ Ä ÀÄ ÀÄ"
#define LINE_DISP CH_DISP CH_DISP CH_DISP "³xxx³\n"
#define CH_WIDTH 24
#define POS_FLD(x) CH_WIDTH*(x)+1
#define ACT_FLD(x) POS_FLD(x)+6
#define INK_FLD(x) ACT_FLD(x)+5
#define INL_FLD(x) INK_FLD(x)+3
#define ZERO_FLD(x) INL_FLD(x)+2
#define OUTL_FLD(x) INL_FLD(x)+3
#define OUTK_FLD(x) OUTL_FLD(x)+3
#define HYST_FLD POS_FLD(3)
#define EOL_FLD HYST_FLD+4

char obuf[] = LINE_DISP;

#define INITIALIZE_DISPLAY show_help(); assert(strlen(obuf)==EOL_FLD+1)
#define PREPARE_FOR_OUTPUT { if (obuf[EOL_FLD] == '\r') putchar('\n');\
			     display_status |= DST_OUTPUT; }
#define UPDATE_OUTPUT printf("%s", obuf)

void dec5(char *p, unsigned short v) {
  p[4] = (v % 10) + '0';
  v /= 10;
  if (v == 0) goto zero4;
  p[3] = (v % 10) + '0';
  v /= 10;
  if (v == 0) goto zero3;
  p[2] = (v % 10) + '0';
  v /= 10;
  if (v == 0) goto zero2;
  p[1] = (v % 10) + '0';
  v /= 10;
  if (v == 0) goto zero1;
  p[0] = v + '0';
  goto zero0;
  zero4: p[3] = ' ';
  zero3: p[2] = ' ';
  zero2: p[1] = ' ';
  zero1: p[0] = ' ';
  zero0: return;
}

void dec3(char *p, unsigned short v) {
  if (v > 999) {
    p[2] = p[1] = p[0] = '*';
    return;
  }
  p[2] = (v % 10) + '0';
  v /= 10;
  if (v == 0) goto zero2;
  p[1] = (v % 10) + '0';
  v /= 10;
  if (v == 0) goto zero1;
  p[0] = v + '0';
  goto zero0;
  zero2: p[1] = ' ';
  zero1: p[0] = ' ';
  zero0: return;
}

void read_data(void) {
  int ch;
  unsigned short value, diff;

  for (ch = 0; ch < N_CHANNELS; ch++) {
    value = crnt.channels[ch].position = RDSUB(UP_DOWN(ch));
    if ((display_status & DST_INITIALIZE) ||
        value != dspd.channels[ch].position) {
      dspd.channels[ch].position = value;
      dec5(obuf+POS_FLD(ch), value);
      display_status |= DST_OUTPUT;
    }
    value = crnt.channels[ch].hw_status = ~RDSUB(STATUS(ch));
    diff = value ^ dspd.channels[ch].hw_status;
    if ((display_status & DST_INITIALIZE) || (diff & (HW_DIR | HW_DONE))) {
      if (value & HW_DONE) strzcpy(obuf+ACT_FLD(ch), "Off");
      else if (value & HW_DIR) strzcpy(obuf+ACT_FLD(ch), "In ");
      else strzcpy(obuf+ACT_FLD(ch), "Out");
      update_bits(ch, HW_DIR|HW_DONE, value);
    }
    CHECK_BIT(HW_INL, INL_FLD(ch), "Ù ", "ÄÄ");
    CHECK_BIT(HW_OUTL, OUTL_FLD(ch), " À", "ÄÄ");
    CHECK_BIT(HW_INK, INK_FLD(ch), "ÄÄ", "Ù ");
    CHECK_BIT(HW_OUTK, OUTK_FLD(ch), "ÄÄ", " À");
    CHECK_BIT(HW_ZERO, ZERO_FLD(ch), "Ä", "Å");
  }
  value = crnt.hyst_value = ~RDSUB(HYST_ADDR);
  if ((display_status & DST_INITIALIZE) || value != dspd.hyst_value) {
    dspd.hyst_value = value;
    dec3(obuf+HYST_FLD, value);
    display_status |= DST_OUTPUT;
  }
}

void show_help(void) {
  printf("?     Show the commands\n"
	 "F1    Command Channel 1\n"
	 "F2    Command Channel 2\n"
	 "F3    Command Channel 3\n"
	 "F9    Hysteresis Toggle\n"
	 "F10   Scroll On/Off\n");
}

void read_command(int channel) {
  char ibuf[80];
  int i, c, rel, base;
  long steps;

  printf("Channel %d Move: ", channel+1); fflush(stdout);
  for (i = 0; i < 80; ) {
    c = getchar();
    if (c == EOF || c == '\n' || c == '\r') break;
    if (c != ' ' && c != '\t' && c != '\f')
      ibuf[i++] = c;
  }
  ibuf[i] = '\0';
  i = 0;
  if (ibuf[i] == '+' || ibuf[i] == '-') rel = ibuf[i++];
  else rel = 0;
  if (ibuf[i] == '0') {
    if (ibuf[++i] == 'x' || ibuf[i] == 'X') {
      base = 16;
      i++;
    } else base = 8;
  } else base = 10;
  for (steps = 0;; i++) {
    if (isdigit(ibuf[i])) steps = steps * base + ibuf[i] - '0';
    else if (base == 16 && isxdigit(ibuf[i]))
      steps = steps * base + toupper(ibuf[i]) + 10 - 'A';
    else break;
  }
  if (ibuf[i] != '\0') printf("\nIllegal Command Format\n");
  else {
    unsigned short address, count;

    if (rel == 0) steps -= crnt.channels[channel].position;
    else if (rel == '-') steps = -steps;
    address = CHAN_BASE(channel);
    if (steps < 0) {
      address += 4;
      steps = -steps;
      if (global_status & GST_HYST_ON) address += 2;
    }
    count = (unsigned short) steps;
    WRSUB(address, ~count);
  }
}

int main(void) {
  int i, c;

  printf("%s", TITLE "\n");
  
  if (load_subbus() == 0)
    diag_error(3, "Resident Subbus Library Required");
  
  /* Verify R/W acknowledge on all addresses */
  for (i = 0; i < N_CHANNELS; i++) {
    check_ack(DRIVE_INH(i), 0xFFFF);
    check_ack(DRIVE_IN(i), 0xFFFF);
    check_ack(UP_DOWN(i), 0);
    check_ack(DRIVE_OUT(i), 0xFFFF);
  }
  check_ack(HYST_ADDR, 0);

  /* Verify R/W of all data lines to all U/D ctrs */
  for (i = 0; i < N_CHANNELS; i++)
    check_rw(UP_DOWN(i), 0xFFFF);

  /* Initialize all is */
  for (i = 0; i < N_CHANNELS; i++) {
    WRSUB(DRIVE_OUT(i), 0xFFFF);
    WRSUB(UP_DOWN(i), 0);
  }
  
  INITIALIZE_DISPLAY;
  display_status = DST_OUTPUT | DST_INITIALIZE;

  for (;;) {
    tick_sic();
    set_cmdenbl(1);
    read_data();
    if (display_status & DST_OUTPUT) {
      UPDATE_OUTPUT;
      display_status &= ~(DST_OUTPUT | DST_INITIALIZE);
    }
    if (kbhit()) {
      switch (c = kgetch()) {
	default:
	case '?': /* Help */
	  PREPARE_FOR_OUTPUT;
	  show_help();
	  break;
	case KG_F1:
	case KG_F2:
	case KG_F3:
	  PREPARE_FOR_OUTPUT;
          disarm_sic();
	  read_command(c-KG_F1);
          break;
	case KG_F9: /* Hysteresis On/Off */
	  PREPARE_FOR_OUTPUT;
	  global_status ^= GST_HYST_ON;
	  printf("Hysteresis %s\n", (global_status & GST_HYST_ON) ?
		"On" : "Off");
	  break;
	case KG_F10: /* Scrolling On/Off */
	  display_status |= DST_OUTPUT;
	  if (obuf[EOL_FLD] == '\r') obuf[EOL_FLD] = '\n';
	  else obuf[EOL_FLD] = '\r';
	  break;
	case KG_ESCAPE:
	  PREPARE_FOR_OUTPUT;
	  break;
      }
      if (c == KG_ESCAPE) break;
    }
  }
  disarm_sic();
  return(0);
}
