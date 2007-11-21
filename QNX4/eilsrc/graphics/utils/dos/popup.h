#include <curses.h>

#define ESC 27
#define CR 10
#ifndef BACKSPACE
#ifdef __QNX__
#define BACKSPACE 127
#else
#define BACKSPACE 8
#endif
#endif

/* Paramenters:
    bx: 1=single line box
	2=double line box
	0=no box

    ans_size:
	<0 - adds the "press any key to continue"
	0  - no answer expected, nothing.
	>0 - size of answer expected.
*/

/* returns a popup window */
extern WINDOW *popup_win(int y, int x, char *str, int attr, int bx, int ans_size);
/* displays the popup, can accept input, calls popup_win */
extern int popup_str(int y, int x, char *str, int attr, int box, int answer_size, char *answer);
/* displays a file in a popup */
extern int popup_file(int y, int x, char *fname, int attr, int box, int ysize, int xsize);
