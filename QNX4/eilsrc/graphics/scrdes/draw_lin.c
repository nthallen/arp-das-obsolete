#include <curses.h>
#include <assert.h>
#include "globals.h"
#include <attr.h>
#include "draw_lin.h"
#include "etc.h"

/* line_chars contains descriptive information about the line-drawing
   graphics characters of the IBM graphics character set.
   These characters are in the range 179-219 (decimal), so this list
   begins with the information for character code 179.  Each byte
   is divided up into 4 2-bit fields for the four directions UP,
   DN, LT and RT.  The contents of the each field indicate how many
   lines are heading that direction in the corresponding graphics
   character.  Since single and double line graphics are defined,
   the value in any field may be 0, 1 or 2.  Values of 3 are
   undefined.  As an example, character 119 is the upper left
   corner character for single-line drawings.  This means it has
   one line heading right and one line heading down and would be
   encoded as:
			UP DN LT RT
			00 01 00 01
   or 0x11.
*/

#define UP 6
#define DN 4
#define LT 2
#define RT 0
unsigned char line_chars[] = {
  (1<<UP) | (1<<DN),
  (1<<UP) | (1<<DN) | (1<<LT),
  (1<<UP) | (1<<DN) | (2<<LT),
  (2<<UP) | (2<<DN) | (1<<LT),
  (2<<DN) | (1<<LT),
  (1<<DN) | (2<<LT),
  (2<<UP) | (2<<DN) | (2<<LT),
  (2<<UP) | (2<<DN),
  (2<<DN) | (2<<LT),
  (2<<UP) | (2<<LT),
  (2<<UP) | (1<<LT),
  (1<<UP) | (2<<LT),
  (1<<DN) | (1<<LT),
  (1<<UP) | (1<<RT),
  (1<<UP) | (1<<LT) | (1<<RT),
  (1<<DN) | (1<<LT) | (1<<RT),
  (1<<UP) | (1<<DN) | (1<<RT),
  (1<<LT) | (1<<RT),
  (1<<UP) | (1<<DN) | (1<<LT) | (1<<RT),
  (1<<UP) | (1<<DN) | (2<<RT),
  (2<<UP) | (2<<DN) | (1<<RT),
  (2<<UP) | (2<<RT),
  (2<<DN) | (2<<RT),
  (2<<UP) | (2<<LT) | (2<<RT),
  (2<<DN) | (2<<LT) | (2<<RT),
  (2<<UP) | (2<<DN) | (2<<RT),
  (2<<LT) | (2<<RT),
  (2<<UP) | (2<<DN) | (2<<LT) | (2<<RT),
  (1<<UP) | (2<<LT) | (2<<RT),
  (2<<UP) | (1<<LT) | (1<<RT),
  (1<<DN) | (2<<LT) | (2<<RT),
  (2<<DN) | (1<<LT) | (1<<RT),
  (2<<UP) | (1<<RT),
  (1<<UP) | (2<<RT),
  (1<<DN) | (2<<RT),
  (2<<DN) | (1<<RT),
  (2<<UP) | (2<<DN) | (1<<LT) | (1<<RT),
  (1<<UP) | (1<<DN) | (2<<LT) | (2<<RT),
  (1<<UP) | (1<<LT),
  (1<<DN) | (1<<RT)
};

/* get_line_code returns the code for the specified coordinates of
   the specified window.  If the coordinates are outside the window,
   or the character is not in the line character set, it returns 0.
   I'll use the fact that mvwinch() returns ERR if the coordinates
   are off the screen.  I've thrown in a kluge due to the fact that
   inch returns negative integers if the character is above 127.
*/
unsigned char get_line_code(WINDOW *win, int x, int y) {
  int c;

  if (x >= 0 && x < COLS && y >= 0 && y < LINES) {
    c = mvwinch(win, y, x) & 0xFF;
    if (c >= 179 && c <= 219) return(line_chars[c-179]);
  }
  return(0);
}


/* mode determines whether this line is for real or just for test.
   If it is a test line (mode == 0) it is drawn directly on curscr
   FALSE   and none of the sophisticated line-checking is performed.
   If it is for real, it is drawn on stdscr and line-checking is
   performed.  In either case, the necessary updates are performed.
*/
void draw_line(int mode, int x0, int y0, int x1, int y1, int n_lines, int code) {
  WINDOW *win, *cwin;
  int x, y, dx, dy, i;
  unsigned char line_char, line_code;

  assert(x0 == x1 || y0 == y1);
  win = mode ? stdscr : curscr;
  cwin= (win==stdscr) ? stdcodescr : curcodescr;
  wattrset(win,attributes[code]);
  cwin->fill_attr=0;
  wattrset(cwin,(unsigned char)code);
  if (y0 == y1) {
    switch(n_lines) {
	case LINE_SINGLE: line_char=196; break;
	case LINE_DOUBLE: line_char=205; break;
	case LINE_REGION: line_char=7; break;  /* region box */
	case LINE_SPACE: line_char=32; break; /* space */
	default: line_char=n_lines;
    }
/*    line_char = n_lines == 1 ? 196 : 205; */	/* a horizontal line */
    dx = (x1 > x0) ? 1 : -1;
    dy = 0;
  } else {
     switch(n_lines) {
	case LINE_SINGLE: line_char=179; break;
	case LINE_DOUBLE: line_char=186; break;
	case LINE_REGION: line_char=7; break;
	case LINE_SPACE: line_char=32; break; /* space */
	default: line_char=n_lines;
    }
/*    line_char = n_lines == 1 ? 179 : 186; */	/* a vertical line */
    dx = 0;
    dy = (y1 > y0) ? 1 : -1;
  }
  for (x = x0, y = y0;;) {
    mvwaddch(win, y, x, line_char);
    mvwaddch(cwin, y, x, line_char);
    if (x == x1 && y == y1) break;
    x += dx;
    y += dy;
  }
  if (n_lines<0)
  for (x = x0, y = y0;;) {
    line_code = 0;
    line_code |= (get_line_code(win, x, y-1) & (3<<DN)) << 2;
    line_code |= (get_line_code(win, x, y+1) & (3<<UP)) >> 2;
    line_code |= (get_line_code(win, x-1, y) & (3<<RT)) << 2;
    line_code |= (get_line_code(win, x+1, y) & (3<<LT)) >> 2;
    for (i = 0; i < 40; i++) {
      if (line_code == line_chars[i]) {
        mvwaddch(win, y, x, 179+i);
        mvwaddch(cwin, y, x, 179+i);
	break;
      }
    }
    if (x == x1 && y == y1) break;
    x += dx;
    y += dy;
  }
/*  if (mode) wnoutrefresh(stdscr);*/
}
