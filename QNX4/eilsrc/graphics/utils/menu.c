#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <ctype.h>
#include "attr.h"
#include "menu.h"

struct mdesc mtbl[MAXMENUS];

/* returns the column width for a menu */
int columns(int md) {
int i,max=0,len;
for (i=0;i<mtbl[md].lines;i++) {
  len=strlen(mtbl[md].items[i]);
  if (len>max) max=len;
}
if (mtbl[md].title!=0) {
    len=strlen(mtbl[md].title);
    if (len>max) max=len;
}
return(max);
}



/* initialises a menu returning a menu descriptor number.
returns negative if error. */
int menu_init(title,list,normattr,hiattr,titattr,selchrs,selnum,escchrs,escnum,vert,bx)
char *title,*list; int *selchrs,selnum,*escchrs,escnum,vert,bx;
unsigned char normattr,hiattr,titattr; {

int md,j,k,l;
char buffer[80];
char *pt1,*pt2, *new;

/* find an empty slot in menu table */
	for (md=0;md<MAXMENUS;md++)
	   if (mtbl[md].flag==0) break;
	if (md>MAXMENUS) return(-1);

/* initialise */
	if (title!=0) {
		mtbl[md].title=(char *)malloc(strlen(title)+1);
		strcpy(mtbl[md].title,title);
		mtbl[md].titattr=titattr;
	}
	mtbl[md].flag=1;
	mtbl[md].xtra=0;
	mtbl[md].normattr=normattr;
	mtbl[md].hiattr=hiattr;
	mtbl[md].m=0;
	mtbl[md].last=1;
	mtbl[md].selnum=selnum;
	mtbl[md].escnum=escnum;
	mtbl[md].vert=vert;
	mtbl[md].bx=bx;
	mtbl[md].items=0;
	mtbl[md].codes=0;
	mtbl[md].lines=0;
	mtbl[md].cols=0;
	/* bug fix 7/8/92, allocate your own space */
	mtbl[md].selchrs=(int *)malloc(selnum*sizeof(int));
	mtbl[md].escchrs=(int *)malloc(escnum*sizeof(int));
	memcpy(mtbl[md].selchrs,selchrs,selnum*sizeof(int));
	memcpy(mtbl[md].escchrs,escchrs,escnum*sizeof(int));


/* find lines, cols, and extract codes and strings */
	if (list!=0) {
		pt1=list; j=0;
		new=(char *)malloc(strlen(list)+2);
		pt2=new; l=0;
		while (*pt1!='\0') {
			if (*pt1=='|')
			if (*(pt1+1)=='|') {
				*pt2++='|';
				pt1++;
			}
			else {
				*pt2++='\0';
				l++;
			}
			else {
				*pt2++=*pt1;
			}
			pt1++;
		}
		*pt2='\0';
		mtbl[md].lines=++l;
		mtbl[md].codes=(int *)malloc(l*sizeof(int));
		mtbl[md].items=(char **)malloc(l*sizeof(char *));

/* place strings and return codes */
		pt1=new;
		for (j=0;j<l;j++) {
			k=0;
			mtbl[md].codes[j]=j+1;
			while (*pt1!='\0') {
				if (*pt1=='@')
				if (*(pt1+1)=='@') {
					buffer[k++]='@';
					pt1+=2;
				}
				else {
					mtbl[md].codes[j]=atoi(++pt1);
					pt1+=strlen(pt1);
				}
				else {
					buffer[k++]=*pt1;
					pt1++;
				}
			}
			pt1++;
			buffer[k]='\0';
			mtbl[md].items[j]=(char *)malloc(strlen(buffer)+1);
			strcpy(mtbl[md].items[j],buffer);
		}
	free(new);
	}
	mtbl[md].cols=columns(md);
	return(md);
}




/* display menu */
int menu(md,y,x,text,spc,pos,active)
int md,y,x,active; char *text; int *spc, *pos; {
/* returns code and text of menu item and selchrs char */
int i,t,line=0,col=0,ysize,xsize,b,h;
int sel=0,at=0;
int found;
int pad_y=0,pad_x=0;
int c=-1;

#define REFRESHMENU prefresh(mtbl[md].m,pad_y,pad_x,y,x,y+ysize,x+xsize)
#define HILIGHT mvWrtNAttr(mtbl[md].m,mtbl[md].hiattr,mtbl[md].cols,line,col);
#define DEHILITE mvWrtNAttr(mtbl[md].m,mtbl[md].normattr,mtbl[md].cols,line,col)
#define MOVECURSOR wmove(mtbl[md].m,line,col)
#define LNS line=(mtbl[md].vert)?at+t+b:b
#define CLS col=(mtbl[md].vert)?b:(at+t)*(mtbl[md].cols+1)+b;
#define LINES_COLS LNS; CLS
#define OUT -2


/* setup */
	if (mtbl[md].flag==0) return(0);
	/* title */
	t=(mtbl[md].title)?1:0;
	b=(mtbl[md].bx)?1:0;
	h=(mtbl[md].vert)?0:1;
	/* menu sizes */
	ysize=(mtbl[md].vert)?mtbl[md].lines+t+b+b-1:b+b;
	xsize=(mtbl[md].vert)?mtbl[md].cols+b+b-1:(mtbl[md].lines+t)*(mtbl[md].cols+1)-1+b+b;
	if (y+ysize>=LINES) ysize=LINES-y-1;
	if (x+xsize>=COLS) xsize=COLS-x-1;


/* build menu */
	if (mtbl[md].m==0) {
		if (!h)
		   mtbl[md].m=newpad(mtbl[md].lines+b+b+t,mtbl[md].cols+b+b);
		else
		   mtbl[md].m=newpad(b+b+1,(mtbl[md].cols+1)*(mtbl[md].lines+t)+b+b);
		keypad(mtbl[md].m,TRUE);
		wattrset(mtbl[md].m,mtbl[md].normattr);
		wclear(mtbl[md].m);
		if (t) {
		   mvwaddstr(mtbl[md].m,b,b,mtbl[md].title);
		   mvWrtNAttr(mtbl[md].m,mtbl[md].titattr,mtbl[md].cols,b,b);
		}
		if (b)
		switch(mtbl[md].bx) {
			case 1: box(mtbl[md].m,VERT_SINGLE,HORIZ_SINGLE);
				break;
			default:
			case 2: box(mtbl[md].m,VERT_DOUBLE,HORIZ_DOUBLE);
				break;
		}
		for (i=0;i<mtbl[md].lines;i++)
		   if (!h)
		      mvwaddstr(mtbl[md].m,b+i+t,b,mtbl[md].items[i]);
		   else
		      mvwaddstr(mtbl[md].m,b,(i+t)*(mtbl[md].cols+1)+b,mtbl[md].items[i]);
	}


/* handle keyed input */
	if (c!=-1&&active) c=wgetch(mtbl[md].m);
	if (mtbl[md].last>mtbl[md].lines) mtbl[md].last=mtbl[md].lines;
	if (!mtbl[md].last&&mtbl[md].lines) mtbl[md].last=1;
 	at=mtbl[md].last-1;

	while (1) {
		
		/* check for escape characters */
		for (i=0;i<mtbl[md].escnum;i++)
		   if (c==mtbl[md].escchrs[i]) {
			*spc=mtbl[md].escchrs[i];
		   	if (pos!=0) *pos=at+1;
			c=OUT;
			break;
		   }
		if (i>=mtbl[md].escnum)
		
		/* check for selection characters */
		for (i=0;i<mtbl[md].selnum;i++)
		    if (c==mtbl[md].selchrs[i]) {
		      if (mtbl[md].lines) {
		      	sel=mtbl[md].codes[at];
		      	if (text) strcpy(text,mtbl[md].items[at]);
		      	if (pos!=0) *pos=at+1;
		      	mtbl[md].last=at+1;
		      } else {
			sel=0; pos=0; if (text) strcpy(text,"");
		      }
		      if (spc!=0) *spc=mtbl[md].selchrs[i];
		      c=OUT;
		      break;
		   }

		if (c!=OUT)
		    if (mtbl[md].lines && (!h&&(c==KEY_UP||c==KEY_DOWN))
				||(h&&(c==KEY_LEFT||c==KEY_RIGHT)))
			switch (c) {
				case KEY_LEFT:
				case KEY_UP:
				    at--; if (at<0) at=mtbl[md].lines-1;
				    break;
				case KEY_RIGHT:
				case KEY_DOWN:
				    at++; at%=mtbl[md].lines;
				    break;
			}
		    else {
			    /* typing to selection */
			    found=0;
			    if (c>31 && c<126) {
				if (mtbl[md].lines) {
				for (i=(at+1)%mtbl[md].lines;i!=at;i=(i+1)%mtbl[md].lines) {
				    if (*(strtok(mtbl[md].items[i]," "))==(char)c) {
					/* normattr out last choice */
					found=1;
					DEHILITE;
					MOVECURSOR;
					REFRESHMENU;
					at=i;
					LINES_COLS;
					/* hilite new choice */
					HILIGHT;
					MOVECURSOR;
					REFRESHMENU;
					break;
				    }
				}
				if (!found) beep();
				} else beep();
			    }
		    }

	/* scroll */
	if (!h) {
	   pad_y=(int)(((at+t+b)/(ysize+1))*(ysize+1));
	   if (((pad_y+ysize)>(mtbl[md].lines+t+b+b))||at==mtbl[md].lines-1)
		pad_y=(mtbl[md].lines+t+b+b)-(ysize+1);
	}
	else {
	   pad_x=(int)((((at+1+t)*(mtbl[md].cols+1)+b)/(xsize+1))*(xsize+1));
	   while (pad_x>0&&pad_x%(mtbl[md].cols+1)!=b) pad_x--;
	   if (((pad_x+xsize)>(mtbl[md].lines+t)*(mtbl[md].cols+1)+b+b)||at==mtbl[md].lines-1)
		pad_x=((mtbl[md].lines+t)*(mtbl[md].cols+1)+b+b)-(xsize+1);
	   if (at!=mtbl[md].lines-1) {
		while (pad_x>0&&pad_x%(mtbl[md].cols+1)!=1) pad_x--;
		while (((at+1+t)*(mtbl[md].cols+1)+b)>(xsize+pad_x)) pad_x++;
	   }
	}
	LINES_COLS;
	if (mtbl[md].lines) {
		MOVECURSOR;
		HILIGHT;
	}
	else wmove(mtbl[md].m,b+t,b);
	REFRESHMENU;
	if (mtbl[md].lines) DEHILITE;
	if (c==OUT||!active) break;
	else c=wgetch(mtbl[md].m);
    }
return(sel);
}


/* dynamically add an item to an existing menu */
int menu_add_item(md,str,pos,code) int md; char *str; int pos,code; {
char **newi;
int *newc;
int i;

/* kill the old menu pad */
	if (!mtbl[md].flag) return(0);
	if (mtbl[md].m) delwin(mtbl[md].m);
	mtbl[md].m=0;
	if (strlen(str)>mtbl[md].cols) mtbl[md].cols=strlen(str);

/* if there is no extra storage */
	if (!mtbl[md].xtra) {
		newi=(char **)malloc((mtbl[md].lines+5)*sizeof(char *));
		newc=(int *)malloc((mtbl[md].lines+5)*sizeof(int));
		if (newi&&newc) mtbl[md].xtra=4; else return(0);
		memmove(newi,mtbl[md].items,mtbl[md].lines*sizeof(char *));
		memmove(newc,mtbl[md].codes,mtbl[md].lines*sizeof(int));
		free(mtbl[md].items);
		free(mtbl[md].codes);
		mtbl[md].codes=newc;
		mtbl[md].items=newi;
	}
	else mtbl[md].xtra--;

/* fix position and code */
	if (!pos) pos=1;
	if (pos>mtbl[md].lines)
	   pos=mtbl[md].lines+1;
	mtbl[md].lines++;
	if (!code) code=pos;

/* place string in position */
	i=mtbl[md].lines;
	newi=(char **)malloc((i-pos)*sizeof(char *));
	newc=(int *)malloc((i-pos)*sizeof(int));
	memmove(newi,mtbl[md].items+(pos-1),(i-pos)*sizeof(char *));
	memmove(newc,mtbl[md].codes+(pos-1),(i-pos)*sizeof(int));
	mtbl[md].items[pos-1]=(char *)malloc(strlen(str)+1);
	strcpy(mtbl[md].items[pos-1],str);
	mtbl[md].codes[pos-1]=code;
	memmove(mtbl[md].items+pos,newi,(i-pos)*sizeof(char *));
	memmove(mtbl[md].codes+pos,newc,(i-pos)*sizeof(int));
	if (newi&&newc) {
		free(newi);
		free(newc);
	}
	return(pos);
}



/* dynamically delete an item from existing menu */
int menu_del_item(md,str,code) int md; char *str; int code; {
int pos=0;

/* kill the old menu pad */
	if (!mtbl[md].flag) return(0);
	if (mtbl[md].m) delwin(mtbl[md].m);
	mtbl[md].m=0;

	if (str)
            while (pos<mtbl[md].lines && stricmp(mtbl[md].items[pos],str)) pos++;
	else
	    while (pos<mtbl[md].lines && code!=mtbl[md].codes[pos]) pos++;
	if (pos==mtbl[md].lines) return(0);
	pos++;

	memmove(&mtbl[md].codes[pos-1],&mtbl[md].codes[pos],(mtbl[md].lines-pos)*sizeof(int));
	free(mtbl[md].items[pos-1]);
	memmove(&mtbl[md].items[pos-1],&mtbl[md].items[pos],(mtbl[md].lines-pos)*sizeof(int));

	if (mtbl[md].last>mtbl[md].lines) mtbl[md].last=1;
	mtbl[md].cols=columns(md);
	mtbl[md].lines--;
	mtbl[md].xtra++;
	return(pos);
}


int menu_end(md) int md; {
int i, all=0;
	if (md>=MAXMENUS) return(0);
	if (md<0) {
	   md=0; all++;
	}
/* free up all space */
	do
	if (mtbl[md].flag!=0) {
	       	if (mtbl[md].m) delwin(mtbl[md].m);
		for (i=0;i<mtbl[md].lines;i++)
		   free(mtbl[md].items[i]);
		free(mtbl[md].items);
		mtbl[md].items=0;
		free(mtbl[md].codes);
		mtbl[md].codes=0;
		free(mtbl[md].title);
		mtbl[md].title=0;
		free(mtbl[md].selchrs);
		mtbl[md].selchrs=0;
		free(mtbl[md].escchrs);
		mtbl[md].escchrs=0;
		mtbl[md].lines=0;
		mtbl[md].cols=0;
		mtbl[md].last=0;
       	        mtbl[md].xtra=0;
		mtbl[md].flag=0;
		mtbl[md].normattr=0;
       	        mtbl[md].hiattr=0;
		mtbl[md].titattr=0;
		mtbl[md].bx=0;
		mtbl[md].vert=0;
		mtbl[md].escnum=0;
		mtbl[md].selnum=0;
		}
	while (all && ++md<MAXMENUS);
return(1);
}

int menu_set_default(int md,char *str,int code) {
int pos=0;
	if (mtbl[md].flag==0) return(0);
	if (mtbl[md].lines==0) return(0);
	if (str) {
	     if (*str=='+'&&*(str+1)!='+')
		pos=mtbl[md].last+atoi(str+1)-1;
	     else if (*str=='-'&&*(str+1)!='-')
		pos=mtbl[md].last-atoi(str+1)-1;
	    else
	        while (pos<mtbl[md].lines && stricmp(mtbl[md].items[pos],str)) pos++;
	}
	else
	    while (pos<mtbl[md].lines && code!=mtbl[md].codes[pos]) pos++;
	pos++;
	if (!pos) pos=mtbl[md].lines;
	if (pos>mtbl[md].lines) pos=1;
	mtbl[md].last=pos;
	return(1);
}

int menu_set_attrs(int md, unsigned char *menu_norm, 
		   unsigned char *menu_hi, unsigned char *menu_title) {
    if (mtbl[md].flag) {
	if (menu_norm) mtbl[md].normattr=*menu_norm;
	if (menu_hi) mtbl[md].hiattr=*menu_hi;
	if (menu_title) mtbl[md].titattr=*menu_title;
	if (mtbl[md].m) delwin(mtbl[md].m);
	mtbl[md].m=0;
	return(1);
    }
    return(0);
}

int menu_set_code(int md, char *item, int code) {
int i;
if (!mtbl[md].flag) return(0);
for (i=0;i<mtbl[md].lines;i++)
    if (!strcmp(mtbl[md].items[i],item)) {
	mtbl[md].codes[i]=code;
	return(1);
    }
return(0);
}

int menu_get_width(int md) {
int w;
if (!mtbl[md].flag) return(0);
w=mtbl[md].cols;
if (!mtbl[md].vert) {
    w+=1;
    w *= (mtbl[md].title) ? mtbl[md].lines+1 : mtbl[md].lines;
}
if (mtbl[md].bx) w+=2;
return(w);
}

int menu_get_length(int md) {
int l;
if (!mtbl[md].flag) return(0);
if (mtbl[md].vert) {
    l=mtbl[md].lines;
    if (mtbl[md].title) l++;
}
else l=1;
if (mtbl[md].bx) l+=2;
return(l);
}
