#include <curses.h>

/* writes attr at (x,y) n times */
void mvWrtNAttr(WINDOW *mw, int attr, int n, int y, int x) {
  int i;
  char *cp;
  cp = mw->scr_image + y*mw->_bufwidth*2 + x*2 + 1;
  for (i = n; i > 0; i--, cp += 2) *cp = attr;
}

/* writes char at (x,y) n times */
void mvWrtNChars(WINDOW *mw, int c, int n, int y, int x) {
  int i;
  char *cp;
  cp = mw->scr_image + y*mw->_bufwidth*2 + x*2;
  for (i = n; i > 0; i--, cp += 2) *cp = c;
}

/* change every occurrence of the attribute 'from' to 'to' in 'win'
   in the rectangle posy, posx, lines, cols */
int chgattr(WINDOW *win, unsigned char from, unsigned char to, int posy, int posx, int lines, int cols) {
   char *p;
   int i;
   WINDOW *swin;
   if ( (swin=newwin(lines,cols,posy,posx))==NULL) return(0);
   overwrite(win,swin);
   p = swin->scr_image+1;
   for (i=0; i<lines*cols; i++, p+=2)
	if (*p==from) *p=to;
   delwin(swin);
   return(1);
}


/* change the attributes in 'win' to the attributes specified in 'attrtab'
   indexed by the attribute codes in the window 'cwin'. */
int WrtAttrTable(WINDOW *win, WINDOW *cwin, unsigned char *attrtab,int posy, int posx, int lines, int cols) {
   int i;
   unsigned char *p, *c;
   WINDOW *swin, *scwin;
   if ( (swin=newwin(lines,cols,posy,posx))==NULL) return(0);
   if ( (scwin=newwin(lines,cols,posy,posx))==NULL) return(0);
   overwrite(win,swin);
   overwrite(cwin,scwin);
   c = scwin->scr_image+1;
   p = swin->scr_image+1;
   for (i=0; i<lines*cols; i++, p+=2, c+=2) *p=attrtab[*c];
   overwrite(swin,win);
   delwin(swin);
   delwin(scwin);
   return(1);
}

