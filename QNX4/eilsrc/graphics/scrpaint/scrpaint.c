#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <curses.h>
#include <curses_utils.h>
#include <eillib.h>

/* globals */
char *opt_string=OPT_MSG_INIT;
unsigned char attributes[MAX_ATTRS];
#define HDR "paint"

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
char space2[NAME_MAX];
extern int optind;

msg_init_options(HDR,argc,argv);
msg(MSG,"task %d: started, and will terminate shortly",getpid());
shdr.nattrs=0;
initscr();
scrollok(stdscr,FALSE);
stdcodescr=newwin(LINES,COLS,0,0);
/* first check for .scr or .fld extensions */
if (strstr(argv[optind],".scr"))
    scr_win_in(argv[optind], stdcodescr, 0, -1, -1, &shdr);
else if (strstr(argv[optind],".fld"))
    fld_win_in(argv[optind], stdcodescr, 0, 0, 0, -1, -1, &shdr);
else
    if ( !scr_win_in(argv[optind], stdcodescr, 0, -1, -1, &shdr) )
	if ( !fld_win_in(argv[optind], stdcodescr, 0, 0, 0, -1, -1, &shdr) )
	    if ( !scr_win_in(extfilename(argv[optind],"scr",space2), stdcodescr, 0, -1, -1, &shdr) )
		fld_win_in(extfilename(argv[optind],"fld",space2), stdcodescr, 0, 0, 0, -1, -1, &shdr);
if (shdr.nattrs) {
    if (argc > optind+1) {
	if (!init_attrs(argv[optind+1],attributes,shdr.nattrs))
	    if (!init_attrs(extfilename(argv[optind+1],"cfg",space2),attributes,shdr.nattrs))
		msg(MSG_FAIL,"error reading cfg of %s",argv[optind+1]);
    }
    else init_attrs(extfilename(argv[optind],"cfg",space2),attributes,shdr.nattrs);
    overwrite(stdcodescr,stdscr);
    WrtAttrTable(stdscr,stdcodescr,attributes,-1,-1,shdr.rows,shdr.cols);
    refresh();
} else msg(MSG_FAIL,"error reading scr or fld of %s",argv[optind]);
delwin(stdcodescr);
term_restore();
term_cur(LINES-1,0);
endwin();
}
