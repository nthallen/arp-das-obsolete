#include "curses.h"
#include <sys/qnxterm.h>
#include <string.h>
#include <stdio.h>


/* Copyright (C) 1989 by Lattice, Inc. Lombard, IL */

/* extern struct _console_ctrl *cc;*/

int
doupdate()

/* This function sends the 'curscr' window to the actual physical	*/
/* screen with an OS/2 call.  It also moves the physical cursor to	*/
/* the current position of the 'curscr' screen.				*/

{
/*  int		chars_to_refresh; # of chars to write to screen */
  register int i,j;
  char *p, *a;

p = curscr->scr_image;
a = absscr;
    
/*  chars_to_refresh = curscr->_maxy * curscr->_maxx * 2;
fprintf(tst, "DOUPDATE: chars: %dPtr: %p\n", chars_to_refresh, curscr->scr_image );
	for(i=0; i<2000; i++)
	{
		fprintf(tst, "[%d] # %d, %d, '%c'\n", i, *p, *(p+1), *p );
		p+=2;
	}
*/

switch (spc) {
case 2:
	/* attribute bytes the same as dos */
	for (j=0; j<curscr->_maxy; j++) {
		if (memcmp(a, p, curscr->_maxx * 2))
			term_restore_image(j,0,p,curscr->_maxx);	
		p+=(curscr->_maxx*2);
		a+=(curscr->_maxx*2);
	}
	break;
default:
	/* this is too bloody slow */
	for (i=0;i<curscr->_maxy;i++)
		for (j=0;j<curscr->_maxx;j++) {
			if (memcmp(a, p, 2))
				term_type(i,j,p,1,table[(unsigned char)*(p+1)]);
			p+=2;
			a+=2;
		}
	break;
}
  mvcur( 0, 0, curscr->_cury, curscr->_curx );
  term_flush();
  memcpy(absscr, curscr->scr_image, curscr->_maxy * curscr->_maxx * 2);
  return( OK );
}


