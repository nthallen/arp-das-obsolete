#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "fldglobs.h"
#include "bufglobs.h"
#include "txtglobs.h"
#include "etc.h"
#include "attr.h"
#include "hdr.h"
#include "fld.h"
#include "tabs.h"
#include "draw_lin.h"

void handle_region(int draw, int n_lines, int code, int r_h_x, int r_h_y, int r_e_x, int r_e_y) {
int i,j;
char buf[80];
int t_l, t_r, t_t, t_b;
int t_c_x, t_c_y;

/* draw region box */
if (region==START_REGION) {
	DRAWBOX(0,r_h_x,r_h_y,r_e_x,r_e_y,LINE_REGION,code);
	return;
}

if (r_h_x<r_e_x) { t_l=r_h_x; t_r=r_e_x; }
else { t_l=r_e_x; t_r=r_h_x; }
if (r_h_y<r_e_y) { t_t=r_h_y; t_b=r_e_y; }
else { t_t=r_e_y; t_b=r_h_y; }
switch(draw) {
case LINE:
	if (r_h_y==r_e_y || r_h_x==r_e_x)
	   draw_line(0,r_h_x,r_h_y,r_e_x,r_e_y,n_lines,code);
	else {
	   DRAWBOX(0,r_h_x,r_h_y,r_e_x,r_e_y,n_lines,code);
	}
break;
case SCREEN:
	if (c==INS) {
	   c_lines=t_b-t_t+1;
	   c_cols=t_r-t_l+1;
	   c_posx=t_l; c_posy=t_t;
	   DRAWBOX(0,t_l,t_t,t_r,t_b,n_lines,code);
	}
	else
	 { DRAWBOX(0,t_l,t_t,t_r,t_b,n_lines,code); }
break;
case TEXT:
	switch (n_lines) {
	case 0:
	break;
	case OPTION1:
		if (r_h_x==r_e_x&&r_h_y==r_e_y) /* one point region */
			t_r=c_posx+c_cols-1;
	break;
	case OPTION2:
		if (r_h_x==r_e_x&&r_h_y==r_e_y)  { /* one point region */
			if (r_h_x<c_posx+c_cols-1-r_h_x) {
				t_l=c_posx;
				t_r=2*r_h_x;
			}
			else {
				t_l=r_h_x-(c_posx+c_cols-1-r_h_x);
				t_r=c_posx+c_cols-1;
			}
		}
	break;
	case OPTION3:
		if (r_h_x==r_e_x&&r_h_y==r_e_y) /* one point region */
		        t_l=c_posx;
	break;
	case OPTION4:
	    if(txtfp) {
	    while (fgets(buf,80,txtfp)&&(strlen(buf)+strlen(txt)+1)<=txtmax) {
		/* what about tabs? */
		tabs_to_space(buf);
	    	strcat(txt,buf);
		buf[0]='\0';
	    }
	    if (strlen(buf)) {
	    	buf[txtmax-strlen(txt)-1]='\0';
		strcat(txt,buf);
	    }
	    fclose(txtfp); txtfp=0;
	    }
	break;
	}
	numlines=t_b-t_t+1;
	linemax=t_r-t_l+1;
	txtmax=linemax*numlines;
	if (c!=INS)
	   { DRAWBOX(0,t_l,t_t,t_r,t_b,LINE_REGION,code); }

	switch ((int)c_txt[0]) {
	case BACKSPACE:
	    if (txtptr!=txt) {
		txtptr--;
		*txtptr='\0';
	    }
	    break;
	case CR:
	    strcat(txt,"\n");
	    txtptr++;
	    break;
	case TAB:
	    c_txt[0]=SPACE;
	    strcat(txt,(char *)c_txt);
	    txtptr++;
	    break;
	case 0:
	    break;
	default:
	    if (isprint((int)c_txt[0])) {
		strcat(txt,(char *)c_txt);
		txtptr++;
	    }
	    break;
	} /* switch */

	txtsiz=strlen(txt);
	/* go line by line through txt and justify */
	t_c_x=t_l; t_c_y=t_t;
	if (dblmode) t_c_y=t_t+(t_b-t_t)/2;
	txtptr=0; i=0;
	wattrset(curscr,attributes[code]);
	curcodescr->fill_attr=0;
	wattrset(curcodescr,(unsigned char)code);
	do {
		if (txtptr==0) txtptr=txt;
		else if (*txtptr=='\n'||i!=0) txtptr++;
		if (*txtptr=='\n'||i>=linemax||*txtptr=='\0') {
			txtline[i]='\0';
			linelen=strlen(txtline);
			if (t_c_y<=t_b) {
				if (n_lines==OPTION3)
				t_c_x=t_l+linemax-linelen;
				else if (n_lines==OPTION2)
				t_c_x=(int)(t_l+(t_r-t_l-linelen+1)/2);
				for (i=0;i<strlen(txtline); i++)
					if (txtline[i]=='\t') txtline[i]=' ';
				mvwaddstr(curscr,t_c_y,t_c_x,txtline);
				mvwaddstr(curcodescr,t_c_y++,t_c_x,txtline);
			}
			else {
				if (txtline[0]!='\0') {
					beep();
					txtptr-=linelen;
					*txtptr='\0';
				}
				else if (*(txtptr-1)=='\n') {
					beep();
					txtptr--;
					*txtptr='\0';
				}
			}
			i=-1;
		}
		else
		txtline[i]=*txtptr;
		i++;
	}
	while (*txtptr!='\0');
	c_txt[0]=0;
break;
case BUFFER:
/* put into a win of its own */
	if (!putbuffer) {
		delwin(bufwin);
		delwin(cpbufwin);
		delwin(blankbufwin);
		delwin(bufcodewin);
		bufwin=newwin(ABS(r_e_y-r_h_y)+1,ABS(r_e_x-r_h_x)+1,r_h_y<r_e_y?r_h_y:r_e_y, r_h_x<r_e_x?r_h_x:r_e_x);
		cpbufwin=newwin(ABS(r_e_y-r_h_y)+1,ABS(r_e_x-r_h_x)+1,r_h_y<r_e_y?r_h_y:r_e_y, r_h_x<r_e_x?r_h_x:r_e_x);
		blankbufwin=newwin(ABS(r_e_y-r_h_y)+1,ABS(r_e_x-r_h_x)+1,r_h_y<r_e_y?r_h_y:r_e_y, r_h_x<r_e_x?r_h_x:r_e_x);
		blankcodewin=newwin(ABS(r_e_y-r_h_y)+1,ABS(r_e_x-r_h_x)+1,r_h_y<r_e_y?r_h_y:r_e_y, r_h_x<r_e_x?r_h_x:r_e_x);
		bufcodewin=newwin(ABS(r_e_y-r_h_y)+1,ABS(r_e_x-r_h_x)+1,r_h_y<r_e_y?r_h_y:r_e_y, r_h_x<r_e_x?r_h_x:r_e_x);
		wattrset(blankbufwin,0);
		wclear(blankbufwin);
		wclear(blankcodewin);
		mvWrtNAttr(blankcodewin,0,(t_b-t_t+1)*(t_r-t_l+1),0,0);
		overwrite(stdscr,bufwin);
		overwrite(stdscr,cpbufwin);
		overwrite(stdcodescr,bufcodewin);
		if (n_lines==OPTION3||n_lines==OPTION4) {
			wnoutrefresh(blankbufwin);
			overwrite(curcodescr,blankcodewin);
		}
		putbuffer=1;
	} else if (region==NO_REGION) {
		mvwin(cpbufwin,c_y,c_x);
		mvwin(bufcodewin,c_y,c_x);
		switch(n_lines) {
			case OPTION1:
			    wnoutrefresh(bufwin);
			    overwrite(cpbufwin,curscr);
			    overwrite(bufcodewin,curcodescr);
			    break;
			case OPTION2:
			    wnoutrefresh(bufwin);
			    overlay(cpbufwin,curscr);
			    overlay(bufcodewin,curcodescr);
			    break;
			case OPTION3:
			    wnoutrefresh(blankbufwin);
			    overwrite(blankcodewin,curcodescr);
			    wnoutrefresh(cpbufwin);
			    overwrite(bufcodewin,curcodescr);
			    break;
			case OPTION4:
	  		    wnoutrefresh(blankbufwin);
			    overwrite(blankcodewin,curcodescr);
			    overlay(cpbufwin,curscr);
			    overlay(bufcodewin,curcodescr);
			break;
		}
	}
if (region==TEST_REGION)
	 { DRAWBOX(0,r_h_x,r_h_y,r_e_x,r_e_y,LINE_REGION,code); }
break;
case FIELD:
	if (c==INS && n_lines==OPTION1) {
	    /* add to field list */
	    fld_add(&fldtop,fldnum,fldtxt,t_t-c_posy,t_l-c_posx,t_r-t_l+1,t_b-t_t+1,code);
	    for (i=0;i<ABS(r_e_y-r_h_y)+1;i++)
		draw_line(0,t_l,t_t+i,t_r,t_t+i,LINE_SPACE,code);
	}
	else if (n_lines==OPTION3) {
	    if (putfield==1 || putfield==2)
		for (i=0;i<fldptr->length;i++)
		    draw_line(0,c_x,c_y+i,c_x+fldptr->width-1,c_y+i,LINE_SPACE,fldptr->attrcode);
	    if (putfield==2) {
		fldptr->line=c_y;
		fldptr->col=c_x;
		putfield=0;
	    }
	}
	if (region==TEST_REGION)
	    { DRAWBOX(0,r_h_x,r_h_y,r_e_x,r_e_y,LINE_REGION,code); }
break;
case ERASE:
	for (i=0;i<ABS(r_e_y-r_h_y)+1;i++)
	        switch(n_lines) {
		case OPTION2: j=0;
		case OPTION3: if (n_lines==OPTION3) j=code;
			mvWrtNAttr(curscr, attributes[j], ABS(r_e_x-r_h_x)+1, t_t+i, t_l);
			mvWrtNAttr(curcodescr, j, ABS(r_e_x-r_h_x)+1, t_t+i, t_l);
		break;
		case OPTION1:
			mvWrtNChars(curscr, SPACE, ABS(r_e_x-r_h_x)+1, t_t+i, t_l);
			mvWrtNChars(curcodescr, SPACE, ABS(r_e_x-r_h_x)+1, t_t+i, t_l);
		break;
		case OPTION4: draw_line(0,t_l,t_t+i,t_r,t_t+i,n_lines,0);
		}
	break;
case FILL:
	if (dblmode)
	    for (i=0;i<ABS(r_e_x-r_h_x)+1;i++)
	      draw_line(0,t_l+i,t_t,t_l+i,t_b,n_lines,code);
	else
	for (i=0;i<ABS(r_e_y-r_h_y)+1;i++)
	   draw_line(0,t_l,t_t+i,t_r,t_t+i,n_lines,code);

/*if (c!=INS && draw==ERASE)
	 { DRAWBOX(0,r_h_x,r_h_y,r_e_x,r_e_y,LINE_REGION,code); }
 */
break;
default:;
}
}


void setup_text(int r_h_x,int r_h_y,int r_e_x,int r_e_y) {
    /* set up text buffers */
	if (txt) free(txt);
	if (txtline) free(txtline);
	numlines=ABS(r_e_y-r_h_y)+1;
	if (r_h_x==r_e_x&&r_h_y==r_e_y)
		linemax=txtmax=c_posx+c_cols-1;
	else {
		linemax=ABS(r_e_x-r_h_x)+1;
		txtmax=linemax*numlines;
	}
	txt=(char *)malloc(txtmax+1);
	txtline=(char *)malloc(linemax+1);
	*txt='\0';
	txtptr=txt;
	c_txt[0]=c_txt[1]='\0';
}
