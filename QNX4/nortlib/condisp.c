/* condisp.c defined nlcon_display() */
#include <stdlib.h>
#include "nortlib.h"
#include "nl_cons.h"
char rcsid_condisp_c[] =
	"$Header$";

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
	  for (i = 0; *s != '\0' && *s != '\n' && i < w; s++) {
		buf[i++] = *s;
		buf[i++] = attr;
	  }
	  console_write(nlcd->con_ctrl, 0, (row*nlcd->columns + col) * 2,
					buf, i, NULL, NULL, NULL);
	}
  }
}
/*
=Name nlcon_display(): Display text on one of several consoles
=Subject Nortlib Console Functions
=Synopsis

#include "nl_cons.h"
void nlcon_display(unsigned int index, int row, int col,
	const char *s, char attr);

=Description

  nlcon_display() displays the specified text using the specified
  attribute at the specified row and column on the specified
  console. The index argument is the index of the console as
  defined by the command line arguments to =Con_init_options=().

=Returns

  Nothing.

=SeeAlso

  =Nortlib Console Functions=, =Con_init_options=().
  
=End
*/
