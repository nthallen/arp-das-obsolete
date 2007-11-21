#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "attr.h"
#include "hdr.h"
#include "fld.h"
#define SPACE 32


/* there is gonna be a problem if someone actually wants to put a double quote
	on the screen, oh well!
*/

int fld_win_out(char * filename,WINDOW *codescr,struct fldinfo *top,
		char **attrtypes, int numattr, int lines, int cols, int posy,
		int posx) {
  int i,j,k,c, strt=0;
  int fattr;
  FILE *fp;
  struct fldinfo *p;
  char buf[80], *w;
  /* open filename */
  if ((fp=fopen(filename,"w"))==0) return(0);
  fputs("/* form: lines, cols, pos_y, pos_x */\n",fp);
  fprintf(fp,"#FORM# %d %d %d %d\n",lines,cols,posy,posx);
  fputs("/* attribute type */\n",fp);
  fputs("#ATTRIBUTES# ",fp);
  for (i=0;i<numattr;i++)
    fprintf(fp,"%s ",attrtypes[i]);
  if (i) fputs("\n",fp);

  /* output field info */
  fputs("/* fields: number, line, col, width, length, attribute code, string */\n",fp);
  p=top;
  while (p) {
    fprintf(fp,"#FIELD# %u %u %u %u %u %d \"",
	    p->num, p->line,p->col,p->width,p->length,p->attrcode);
    fprintf(fp,"%s\"\n",p->txt);
    p=p->fldnext;
  }

  /* output form info: background, size, posx, posy and strings */
  fputs("/* strings: line, col, attribute code, string */\n",fp);
  w=codescr->scr_image; k=0; c=0;
  for (i=0;i<lines;i++) {
    fattr=-1;
    for (j=0;j<cols;j++, w+=2)
      if ((*w!=SPACE || *(w+1)!=0) && j!=cols-1 )
	if (*(w+1)!=fattr) {
	  if (strt) {
	    buf[k]='\0';
	    fprintf(fp,"#STRING# %d %d %d \"%s\"\n",i,c,fattr,buf);
	  }
	  strt=1;
	  k=0; c=j;
	  fattr=*(w+1);
	  buf[k++]=*w;
	}
	else buf[k++]=*w;
      else {
	/* end buffer if buf exists */
	if (strt) {
	  buf[k]='\0';
	  fprintf(fp,"#STRING# %d %d %d \"%s\"\n",i,c,fattr,buf);
	}
	strt=0;
	fattr=-1;		/* bug fix # 1, 10/16/91 */
      }
  }

  fclose(fp);
  return(1);
}


/* if actually collecting fields into 'top' then 'numflds' will hold
    the highest field number, else just the number of fields.
*/
int fld_win_in(char *filename, WINDOW *win, struct fldinfo **top,
	       int *numflds, char **attrtypes, int posy, int posx,
	       struct hdr *fldhdr) {
  char buffer[200], *p, *b, buf[80];
  int commcount, i,j,k;
  int numattrs;
  struct fldinfo *f;
  FILE *fp;
  WINDOW *inwin;
  struct hdr fhdr;

  /* open file */
  if ((fp=fopen(filename,"r"))==0) return(0);
  commcount=0;
  if (numflds) *numflds=0;
  if (top) *top=0;

  /* read a line */
  while (fgets(buffer, sizeof(buffer), fp)) {
    /* disregard if it is a comment, increment comment count */
    if (buffer[0]=='/' && buffer[1]=='*') commcount++;
    else
      /* scan into vars */
      switch (commcount) {
      case 1:
	/* read lines, cols, pos_y, pos_x */
	/* form: lines, cols, pos_y, pos_x */
	if (sscanf(buffer,"%*s %d %d %d %d",
		  &fhdr.rows,&fhdr.cols,&fhdr.pos_y,&fhdr.pos_x)!=4) return(0);
	posy=(posy<0)?fhdr.pos_y:posy;
	posx=(posx<0)?fhdr.pos_x:posx;
	if ( (inwin=newwin(fhdr.rows,fhdr.cols,posy,posx))==NULL) return(0);
	inwin->fill_attr=0;
	wclear(inwin);
	break;
      case 2:
	/* build attribute types */
	for (numattrs=0,b=buffer+13;(p=strtok(b," \n"))!=NULL;numattrs++) {
	  if (attrtypes) {
	    attrtypes[numattrs]=(char *)malloc(strlen(p)+1);
	    strcpy(attrtypes[numattrs],p);
	  }
	  b=NULL;
	}
	fhdr.nattrs=numattrs;
	break;
      case 3:
	/* build field list */
	/* fields: number, line, col, width of field, attribute code, string */
	if (top) {
	  if (!*top) {
	    f=(struct fldinfo *)malloc(FLDINFOSZ);
	    *top=f;
	  }
	  else {
	    f->fldnext=(struct fldinfo *)malloc(FLDINFOSZ);
	    f=f->fldnext;
	  }
	  /* bug #2 : must read everything between 2 quotes, even spaces */
	  if (sscanf(buffer,"%*s %d %d %d %d %d %d \"%[^\"\n]",
		     &f->num,&f->line,&f->col,&f->width,&f->length,
		     &f->attrcode,buf) < 6) return(0);
	  if (numflds && f->num>*numflds) *numflds=f->num;
	  f->txt=(char *)malloc(strlen(buf)+1);
	  strcpy(f->txt,buf);
	  f->fldnext=0;
	  /* bug #3, add the attribute */
	  for (i=f->line;i<f->line+f->length;i++)
	    mvWrtNAttr(inwin,f->attrcode,f->width,i,f->col);
	} else if (numflds) *numflds++;
	break;
      case 4:
	/* place strings */
	/* strings: line, col, attribute code, string */
	/* bug #2 : must read everything between 2 quotes, even spaces */
	if (sscanf(buffer,"%*s %d %d %d \"%[^\"\n]",&i,&j,&k,buf)!=4)
	  return(0);
	inwin->fill_attr=0;
	wattrset(inwin,k);
	mvwaddstr(inwin,i,j,buf);
	break;
      }
  }
  if (fldhdr) *fldhdr=fhdr;
  overwrite(inwin,win);
  delwin(inwin);
  fclose(fp);
  return(numattrs);
}


/* ex and ey can be negative, they wont be taken into account */
int fld_num(struct fldinfo *f, int cy, int cx, int ey, int ex,
	    struct fldinfo **info) {
  struct fldinfo *p;

  p=f;
  while (p)
    if ( (cy<=p->line && cx<=p->col && (ey==-1 || (ey>=p->line+p->length-1 &&
						   ex>=p->col+p->width-1)))
	|| ( cy>=p->line+p->length-1 && cx>=p->col+p->width-1 &&
	    (ex==-1 || (ex<=p->col && ey<=p->line))) )  {
      if (info) *info=p;
      return(p->num);
    } else  p=p->fldnext;

  return(-1);
}

int fld_add(struct fldinfo **fldtop,int num,char *txt,int line, int col,
	    int width,
	    int length, int attrcode) {
  struct fldinfo *fldptr;

  fldptr=(struct fldinfo *)malloc(sizeof(struct fldinfo));
  if (!fldptr) return(0);
  fldptr->fldnext=*fldtop;
  *fldtop=fldptr;
  fldptr->num=num;
  fldptr->txt=malloc( (txt ? strlen(txt) : 0) +1);
  if (!fldptr->txt) return(0);
  strcpy(fldptr->txt, txt ? txt : "");
  fldptr->line=line;
  fldptr->col=col;
  fldptr->width=width;
  fldptr->length=length;
  fldptr->attrcode=attrcode;
  return(1);
}


int fld_del(struct fldinfo **f,int i) {
  struct fldinfo *p, *q;
  p=*f;
  if (p->num==i) {
    *f=p->fldnext;
    free(p->txt);
    free(p);
    return(i);
  }
  while (p->fldnext)
    if (p->fldnext->num==i) {
      q=p->fldnext;
      p->fldnext=q->fldnext;
      free(q->txt);
      free(q);
      return(i);
    }
    else p=p->fldnext;
  return(-1);
}

void fld_free(struct fldinfo *f) {
  struct fldinfo *p, *e;
  p=f;
  while (p) {
    free(p->txt);
    p=p->fldnext;
  }
  p=f;
  while (p) {
    e=p;
    p=p->fldnext;
    free(e);
  }
}
