/* condisp.c defined nlcon_display() */
#include <stdlib.h>
#include "nortlib.h"
#include "nl_cons.h"

/* displays without moving cursor. */
void nlcon_display(unsigned int index, int row, int col,
					const char *s, char attr) {
  nl_con_def *nlcd;
  
  if ( index < MAXCONS ) {
	nlcd = &nl_cons[ index ];
	if ( nlcd->con_ctrl != NULL ) {
	  static char *buf = NULL;
	  static int bufsize = 0;
	  int i, w;

	  if ( bufsize < nlcd->columns ) {
		bufsize = nlcd->columns;
		buf = realloc( buf, bufsize*2 );
		if ( buf == 0 )
		  nl_error( 4, "Insufficient memory in nlcon_display" );
	  }

	  w = ( nlcd->columns - col ) * 2;
	  for (i = 0; *s != '\0' && i < w; s++) {
		buf[i++] = *s;
		buf[i++] = attr;
	  }
	  console_write(nlcd->con_ctrl, 0, (row*nlcd->columns + col) * 2,
					buf, i, NULL, NULL, NULL);
	}
  }
}
