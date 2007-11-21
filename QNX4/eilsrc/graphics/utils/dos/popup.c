#include <stdlib.h>
#include <string.h>
#include "popup.h"
#include "tabs.h"
#ifdef __QNX__
#include <lat.h>
#endif

WINDOW *popup_win(int y, int x, char *str, int attr, int bx, int ans_size) {
int xsize,ysize;
WINDOW *win;
char *s, *t;
int max=0,i,last,b;

/* figure size of window */
b=bx?1:0;

ysize=b+b;
if ( (s=(char *)malloc(strlen(str)+1+1)) == NULL) return(0);
strcpy(s,str);
strcat(s,"\n");
t=strtok(s,"\n");
while (t!=NULL) {
	ysize++;
	if (strlen(t)>max) max=strlen(t);
	last=strlen(t);
	t=strtok(NULL,"\n");
}
xsize=max+b+b+1;
if (ans_size>0&&ans_size+1>max-last) xsize=max+(ans_size+2+(last-max+1));
if (ans_size<0&&xsize<=25) xsize=25+b+b+1;
if (ans_size<0&&!b) ysize++;

/* make a window */
if ( (win=newwin(ysize,xsize,y,x))==NULL) return(0);
keypad(win,TRUE);
wattrset(win,attr);
wclear(win);
switch(bx) {
case 1: box(win,VERT_SINGLE,HORIZ_SINGLE); break;
case 2: box(win,VERT_DOUBLE,HORIZ_DOUBLE); break;
}
strcpy(s,str);
tabs_to_space(s);
strcat(s,"\n");
t=strtok(s,"\n");
i=0;
while (t!=NULL) {
	mvwaddstr(win,b+i,b,t);
	i++;
	t=strtok(NULL,"\n");
}
if (ans_size<0) mvwaddstr(win,ysize-1,(xsize/2-13)<0?0:xsize/2-13,"Press any Key to Continue");
/* display window */
free(s);
if (ans_size) wmove(win,ysize-b-1,last+b+1);

return(win);

}

int popup_str(int y, int x, char *str, int attr, int bx, int ans_size, char *retstr) {

int i,c,b,j;
WINDOW *win;
int ans_x, ans_y;

if (ans_size>0 && !retstr) return(0);
if (retstr) retstr[0]='\0';

if ( !(win=popup_win(y,x,str,attr,bx,ans_size))) return(0);
getyx(win,ans_y,ans_x);

wrefresh(win);

/* direct input to it */
/* get input */
i=0;
if (ans_size) c=wgetch(win);
if (ans_size>0)
	while (c!=CR&&c!=ESC) {
		if (c==BACKSPACE) {
			if (i>0&&i<ans_size) i--;
			else if (i!=0) i=ans_size-1;
		}
		if (i>ans_size-1) beep();
		else {
			if (c!=BACKSPACE)
			   retstr[i++]=(char)c;
			retstr[i]='\0';
			for (j=0;j<ans_size;j++)
				mvwaddch(win,ans_y,ans_x+j,SPACE);
			mvwaddstr(win,ans_y,ans_x,retstr);
			wrefresh(win);
		     }
		c=wgetch(win);
	}

/* cleanup */
delwin(win);
return(1);
}


int popup_file(int y, int x, char *fname, int attr, int bx, int ysize, int xsize) {
FILE *fp;
int b,i;
WINDOW *win, *bxwin;
char buf[80];
/* open file */
if ( (fp=fopene(fname,"r",NULL))==NULL ) return(0);
/* make a window */
if (bx)  { y++; x++; }
win=newwin(ysize,xsize,y,x);
wattrset(win,attr);
wclear(win);
b=bx?1:0;
if (b) {
	bxwin=newwin(ysize+2,xsize+2,y-1,x-1);
	wattrset(bxwin,attr);
	wclear(bxwin);
}
switch(bx) {
	case 1: box(bxwin,VERT_SINGLE,HORIZ_SINGLE); break;
	case 2: box(bxwin,VERT_DOUBLE,HORIZ_DOUBLE); break;
}
if (xsize>24) mvwaddstr(bxwin,ysize+1,(xsize+1)/2-13<0?0:(xsize+1)/2-13,"Press any Key to Continue");
wmove(win,0,0);
if (bx) wrefresh(bxwin);
i=-2;
while (fgets(buf,xsize,fp)) {
	i++;
	if (i==ysize && buf[strlen(buf)-1]=='\n') buf[strlen(buf)-1]='\0';
	waddstr(win,buf);
	if (i==ysize) {
		wmove(win,0,0);
		wrefresh(win);
		wgetch(win);
		i=-2;
		wclear(win);
		buf[0]='\0';
	}
}
wmove(win,0,0);
wrefresh(win);
if (buf[0]!='\0') wgetch(win);
delwin(win);
if (bx) delwin(bxwin);
fclose(fp);
return(1);
}



