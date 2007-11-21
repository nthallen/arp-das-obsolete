#define MAX_ATTRS 256

/* writes attr at (x,y) n times */
extern void mvWrtNAttr(WINDOW *mw, int attr, int n, int y, int x);

/* writes char at (x,y) n times */
void mvWrtNChars(WINDOW *mw, int c, int n, int y, int x);

/* the following functions return positive if successful, else zero */

/* change every occurrence of the attribute 'from' to 'to' in 'win'
   in the rectangle posy, posx, lines, cols */
int chgattr(WINDOW *win, unsigned char from, unsigned char to, int posy, int posx, int lines, int cols);

/* change the attributes in 'win' to the attributes specified in 'attrtab'
   indexed by the attribute codes in the window 'cwin'. */
int WrtAttrTable(WINDOW *win, WINDOW *cwin, unsigned char *attrtab,int posy, int posx, int lines, int cols);
