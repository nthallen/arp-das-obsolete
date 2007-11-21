#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hdr.h"
#include "txt.h"


/* output .txt file from a window, returns success status */
int txt_win_out(char *filename, WINDOW *win, char **attrtypes, int nattrs, int posy, int posx, int lines, int cols) {
FILE *fp;
int i;
char *p;
WINDOW *outwin;
/* open filename */
if ((fp=fopen(filename,"w"))==0) return(0);
/* output window */
outwin=newwin(lines,cols,posy,posx);
overwrite(win,outwin);
p=outwin->scr_image;
for (i=0;i<lines*cols;i++,p+=2) {
	if (fputc(*p,fp)==EOF) return(0);
	if (i && i%(cols-1)==0)
		if (fputc('\n',fp)==EOF) return(0);
}
delwin(outwin);
fclose(fp);
return(1);
}





