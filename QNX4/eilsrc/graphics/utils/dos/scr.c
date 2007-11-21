#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hdr.h"
#include "scr.h"

/* input .scr file to a window, returns number of attributes */
/* negative on posx and posy will default */
int scr_win_in(char *filename, WINDOW *win, char **attrtypes, int posy, int posx, struct hdr *shdr) {
FILE *fp;
int c,i;
char *p, buf[50];
WINDOW *inwin;
int lines,cols,px,py;
/* open filename */
	if ((fp=fopen(filename,"r"))==0) return(0);
/* input header */
	if (fgetc(fp)!=SCRMAGIC) return(0);
	lines=fgetc(fp);
	cols=fgetc(fp);
	py=fgetc(fp);
	px=fgetc(fp);
	posy=(posy<0)?py:posy;
	posx=(posx<0)?px:posx;
	c=fgetc(fp);
	if (shdr) {
		shdr->pos_y=py;
		shdr->pos_x=px;
		shdr->rows=lines;
		shdr->cols=cols;
		shdr->nattrs=c;
	}
/* input windows screen */
	inwin=newwin(lines,cols,posy,posx);
	p=inwin->scr_image;
	for (i=0;i<2*lines*cols;i++)
		*p++=fgetc(fp);
/* input attrtypes table */
	if (attrtypes)
	    for (i=0;i<c;i++) {
		fscanf(fp,"%s",buf);
		attrtypes[i]=(char *)malloc(strlen(buf)+1);
		strcpy(attrtypes[i],buf);
	    }
overwrite(inwin,win);
delwin(inwin);
fclose(fp);
return(c);
}

/* output .scr file from a window, returns success status */
int scr_win_out(char *filename, WINDOW *win, char **attrtypes, int nattrs, int posy, int posx, int lines, int cols) {
FILE *fp;
int i;
char *p;
WINDOW *outwin;
/* open filename */
	if ((fp=fopen(filename,"w"))==0) return(0);
/* output header */
	fputc(SCRMAGIC,fp);
	fputc(lines,fp);
	fputc(cols,fp);
	fputc(posy,fp);
	fputc(posx,fp);
	if (fputc(nattrs,fp)==EOF) return(0);
/* output window */
	outwin=newwin(lines,cols,posy,posx);
	overwrite(win,outwin);
	p=outwin->scr_image;
	for (i=0;i<2*lines*cols;i++)
		if (fputc(*p++,fp)==EOF) return(0);
/* output attribute name/code table */
	for (i=0;i<nattrs;i++) {
		if (fputc(' ',fp)==EOF) return(0);
		fputs(attrtypes[i],fp);
	}
delwin(outwin);
fclose(fp);
return(1);
}





