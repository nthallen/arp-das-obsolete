#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include "scr.h"
#include "fld.h"
#include "hdr.h"
#include "cfg.h"
#include "attr.h"

unsigned char attributes[MAX_ATTRS];

char *extfilename(char *name, char *ext, char *ans) {
char *p;
strcpy(ans,name);
/* if there is an extension already */
if ((p=strchr(ans,(int)'.'))!=0)
	*p='\0';
strcat(ans,".");
strcat(ans,ext);
return(ans);
}

main(int argc, char **argv) {
WINDOW *stdcodescr;
struct hdr shdr;
int i;
char space2[256];
initscr();
scrollok(stdscr,FALSE);
stdcodescr=newwin(LINES,COLS,0,0);
if ( !scr_win_in(extfilename(argv[1],"scr",space2), stdcodescr, 0, -1, -1, &shdr) )
    fld_win_in(extfilename(argv[1],"fld",space2), stdcodescr, 0, 0, 0, -1, -1, &shdr);
if (shdr.nattrs) {
    for (i=0;i<shdr.nattrs;i++) attributes[i]=0x07;
    init_attrs(extfilename(argv[1],"cfg",space2),attributes,shdr.nattrs);
    overwrite(stdcodescr,stdscr);
    WrtAttrTable(stdscr,stdcodescr,attributes,-1,-1,shdr.rows,shdr.cols);
    refresh();
} else printf("error reading scr or fld of %s\n",argv[1]);
delwin(stdcodescr);
term_restore();
term_cur(LINES-1,0);
endwin();
}
