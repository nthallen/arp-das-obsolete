/*
Scrdes: Screen Design Program to design static text screens.
Specifications can be found in the Lotus Agenda file "utils.ag".
Eil Sch, Feb 1991.
*/

/* headers */
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <ctype.h>
#include <curses_utils.h>
#include "version.h"
#include "pattr.h"
#include "etc.h"
#include "help.h"
#ifdef __QNX__
#include "lat.h"
#include <termios.h>
#include <unistd.h>
#endif

/* functions */
extern void setup_text(int,int,int,int);
extern void handle_region(int, int, int, int, int, int, int);
int x_place(int menu, int mpos, int apos);
int y_place(int menu);
int getfiles(char *);
char *extfilename(char *, char *, char *);

static char rcsid[]="$Id$";
/* globals.h */
/* current vars */
char *progname;
int mode=OPTION1;
int dblmode=0;
int c_x,c_y;
int c;
int c_lines;
int c_cols;
int c_posx;
int c_posy;
static int mx,my;
static int px,py;
static int c_screen=-1;
static int c_loaded=-1;
static int c_nattrs;
static int c_nscreens;
/* region vars */
int region=NO_REGION;
/* code vars */
WINDOW *stdcodescr;
WINDOW *curcodescr;
unsigned char attributes[MAX_ATTRS];
char *attrtypes[MAX_ATTRS]={"NORMAL"};

/* pattr.h */
unsigned char pattributes[8];

/* bufglobs.h */
/* buffer vars */
WINDOW *bufwin=0;
WINDOW *cpbufwin=0;
WINDOW *bufcodewin=0;
WINDOW *blankbufwin=0;
WINDOW *blankcodewin=0;
int putbuffer=1;

/* txtglobs.h */
/* text vars */
int c_txt[2];
char *txt=0;
char *txtptr=0;
int txtsiz=0;
int txtmax;
int linemax;
int linelen;
int numlines;
char *txtline=0;
FILE *txtfp;

/* fldglobs.h */
/* fld vars */
struct fldinfo *fldtop=0;
struct fldinfo *fldptr;
char *fldtxt;
int fldnum;
int numflds=0;
int putfield=0;

/* mnuglobs.h */
/* menu vars */
int genmenu;
int attrmenu;
int filemenu;
int drawmenu;
int exitmenu;
int linemenu;
int textmenu;
int bufmenu;
int savemenu;
int prefmenu;
int pattrmenu;
int erasemenu;
int fieldmenu;
int upgen;
int updraw;
int upexit;

/* file vars */
static char node[80];
static char fna[500];
static char *fnptrs[50];
static char *cfna;
static char **cfnptrs;
static int fcnt;

main(argc,argv) int argc; char **argv; {

int i,j,k;	    /* counters */
int up_gwin=1;	    /* general window toggle */
WINDOW *gwin;	    /* general window */
char space1[110];   /* useful space */
char space2[110];
char space3[110];
int sel_keys[10]=   {CR,HELPKEY,KEY_DOWN,KEY_UP,KEY_PPAGE,KEY_NPAGE,DEL};
int esc_keys[10]=   {ESC,TOGGLESTAT,KEY_LEFT,KEY_RIGHT,INS};
char *pattrtypes[8]={"MENU_NORMAL","MENU_HILITE","MENU_TITLE","STATUS_WIN",
		    "HELP_WIN","ERROR_WIN","ASK_WIN","TELL_WIN"};
int foreground;
int background;
char *ans;
char *ptr;
int spc;
struct hdr shdr;
/* msg vars */
char *drawmsgs[]={"line\n","text\n","fill\n","erase\n","buffer\n","screen\n","field\n"};
char *linemsgs[]={"single","double","ascii","color"};
char *textmsgs[]={"left","center","right","import"};
char *fieldmsgs[]={"create","delete","move","info"};
char *erasemsgs[]={"foreground","background","color","all"};
char *buffermsgs[]={"cpy/write","cpy/lay","mov/write","mov/lay"};
#define AMSG "Attribute :            \n"
#define SMSG "   Screen :            \n"
#define FMSG " Filename :            \n"
#define EMSG "  Element :            \n"
#define OMSG "   Option :            \n"
char attrmsg[]= AMSG;
char screenmsg[]= SMSG;
char elmsg[]= EMSG;
char optionmsg[]= OMSG;
char filemsg[]= FMSG;
char wholemsg[25*5+1];
int c_draw;
int c_code;
int r_h_x, r_h_y;
int r_e_x, r_e_y;
#ifdef __QNX__
struct termios option_save, option_set;
#endif

#define CLEARSCREENS \
	clear(); \
	stdcodescr->fill_attr=c_code; \
	curcodescr->fill_attr=c_code; \
	wclear(stdcodescr); \
	wclear(curcodescr)

#define SETMENUATTR \
    for (i=0;i<MAXMENUS;i++) \
	menu_set_attrs(i,&MENU_NORMAL,&MENU_HILITE,&MENU_TITLE)


/* initialisations */
for (i=0;i<MAX_ATTRS;i++) attributes[i]=0x07;
initscr();
if (!init_attrs("scrdes.cfg",pattributes,8))
	if (!init_attrs("color.cfg",pattributes,8))
		if (!init_attrs("mono.cfg",pattributes,8)) {
			ERROR("No program cfg file found");
			for (i=0;i<8;i++) pattributes[i]=0x07;
		}
#ifdef __QNX__
tcgetattr(fileno(stdin),&option_save);
option_set=option_save;
option_set.c_iflag = option_set.c_iflag & ~IXOFF;
option_set.c_iflag = option_set.c_iflag & ~IXON;
tcsetattr(fileno(stdin),TCSANOW,&option_set);
#endif
keypad(stdscr,TRUE);
noecho();
ans=malloc(COLS+1);
fldtxt=malloc(COLS+1);
c_draw=LINE;
c_code=0;
c_nattrs=1;
c_lines=LINES;
c_cols=COLS;
c_posx=0;
c_posy=0;
stdcodescr=newwin(LINES,COLS,0,0);
curcodescr=newwin(LINES,COLS,0,0);
CLEARSCREENS;
move(c_y,c_x);
refresh();
scrollok(stdscr,FALSE);
scrollok(curscr,FALSE);
scrollok(stdcodescr,FALSE);
scrollok(curcodescr,FALSE);
px=COLS/8;
py=LINES/4*3;
spc=0;
c=FIRST;


/* initialise menus */
upgen = updraw = upexit = 0;
sprintf(space1,"line@%d|text@%d|fill@%d|erase@%d|buffer@%d|field@%d|screen@%d",LINE,TEXT,FILL,ERASE,BUFFER,FIELD,SCREEN);
drawmenu=menu_init("CTRL-Draw",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,2,esc_keys,4,1,2);
sprintf(space1,"save@%d|prefer@%d|status@%d|help@%d|keys@%d|ascii@%d|version@%d|quit@%d",SAVE,PREFERENCE,TOGGLESTAT,HELPKEY,KEYS,ASCII,VERS,QUIT);
exitmenu=menu_init("CTRL-Exit",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,2,esc_keys,4,1,2);
sprintf(space1,"single@%d|double@%d|ascii@%d|color@%d",ONE,TWO,THREE,FOUR);
linemenu=menu_init("CTRL-Option",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,2,esc_keys,4,1,1);
sprintf(space1,"left@%d|center@%d|right@%d|import@%d",ONE,TWO,THREE,FOUR);
textmenu=menu_init("CTRL-Option",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,2,esc_keys,4,1,1);
sprintf(space1,"create@%d|delete@%d|move@%d|info@%d",ONE,TWO,THREE,FOUR);
fieldmenu=menu_init("CTRL-fIeld",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,2,esc_keys,4,1,1);
sprintf(space1,"cpy/write@%d|cpy/lay@%d|mov/write@%d|mov/lay@%d",ONE,TWO,THREE,FOUR);
bufmenu=menu_init("CTRL-Option",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,2,esc_keys,4,1,1);
sprintf(space1,"foreground@%d|background@%d|color@%d|all@%d",ONE,TWO,THREE,FOUR);
erasemenu=menu_init("CTRL-Option",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,2,esc_keys,4,1,1);
sprintf(space1,"color@%d|mono@%d|set@%d",CHOICE1,CHOICE2,CHOICE3);
prefmenu=menu_init("CTRL-Pref",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,2,esc_keys,4,1,1);
sprintf(space1,"scr@%d|cfg@%d|fld@%d",CHOICE1,CHOICE2,CHOICE4);
savemenu=menu_init("CTRL-Save",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,2,esc_keys,4,1,1);
sprintf(space1,"FILES@%d|ATTRIBUTES@%d|DRAW@%d|EXIT/ETC@%d",FILES,ATTRIBUTE,DRAW,EXIT);
genmenu=menu_init("CTRL-General",space1,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,3,esc_keys,2,0,1);
attrmenu=menu_init("CTRL-Attributes","NORMAL",MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,7,esc_keys,5,1,1);
pattrmenu=menu_init(0,0,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,6,esc_keys,2,1,1);
sel_keys[4]=DEL;
filemenu=menu_init("CTRL-Files",0,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,5,esc_keys,5,1,1);
sel_keys[4]=KEY_PPAGE;
for (i=0;i<8;i++) menu_add_item(pattrmenu,pattrtypes[i],10000,0);
c_y=LINES/8;
c_x=(COLS-menu_get_width(genmenu))/2;


/* process command line args */
progname=argv[0];
fcnt=0;
cfna=fna;
cfnptrs=fnptrs;
/* build dynamic screen menu */
for (i=1;i<argc;i++)
    if (getfiles(argv[i])<0) {
	ERROR("Too many file names or bad file pattern");
	break;
    }
if (c_screen==-1&&fcnt) c_screen=0;


/* main instruction loop */
while (1)  {   switch (c)   {

/* cursor  keys */
case KEY_DOWN:
	if (c_y+1<c_posy+c_lines)
		c_y++;
	c=0;
break;
case KEY_UP:
	if (c_y-1>=c_posy)
		c_y--;
	c=0;
break;
case KEY_LEFT:
	if (c_x-1>=c_posx)
		c_x--;
	c=0;
break;
case KEY_RIGHT:
	if (c_x+1<c_posx+c_cols)
		c_x++;
	c=0;
break;

/* menu keys */
case GENERAL:
	/* bring up general menu */
	mx=x_place(genmenu,0,0);
	my=y_place(genmenu);
	c=menu(genmenu,my,mx,0,&spc,0,1);
	switch (spc) {
	    case HELPKEY: help_menu(genmenu,c,c_draw); c=GENERAL; break;
	    case TOGGLESTAT: up_gwin=up_gwin?0:1; c=GENERAL; break;
	}
	upgen=(c&&c!=GENERAL)?1:0;
break;
case ATTRIBUTE:
	/* bring up attribute menu */
	menu_set_attrs(attrmenu,0,&(attributes[menu_get_last(attrmenu)-1]),0);
	c=menu(attrmenu,y_place(attrmenu),x_place(attrmenu,2,0),0,&spc,&j,1);
	foreground = attributes[j-1] & 0xF;
	background = (attributes[j-1] >> 4) & 0xF;
	switch (spc) {
	   case KEY_LEFT: menu_set_default(genmenu,0,FILES); break;
	   case KEY_RIGHT: menu_set_default(genmenu,0,DRAW); break;
	   case INS:
		/* adding a new attribute type */
		/* find first available slot at end for code */
		/* get name */
		popup_str(LINES/2,COLS/4,"Enter attribute name:",ASK_WIN,1,12,ans);
		if (strlen(ans)) {
			menu_add_item(attrmenu,ans,10000,0);
			attrtypes[c_nattrs]=(char *)malloc(strlen(ans)+1);
			strcpy(attrtypes[c_nattrs],ans);
			c_code=c_nattrs;
			c_nattrs++;
		}
		c=ATTRIBUTE;
	   break;
	   case CR:
		c_code=j-1;
		c=0;
	   break;
	   case DEL:
		/* deleting an attribute type */
		/* confirm */
		popup_str(LINES/2,COLS/4,"Delete this attribute?",ASK_WIN,1,3,ans);
		if (strchr(strlwr(ans),(int)'y')) {
		/* ask for another attribute type to take its place */
			popup_str(LINES/2,0,"Give another attribute type to replace it:",ASK_WIN,1,12,ans);
			for (i=0;i<c_nattrs;i++)
			    if (!stricmp(ans,attrtypes[i])) break;
			if (i>=c_nattrs) {
				ERROR("Replacement attribute does not exist");
				c=ATTRIBUTE;
				break;
			}
 			chgattr(stdcodescr,(unsigned char)(j-1),(unsigned char)i,c_posy,c_posx,c_lines,c_cols);
			c_code=i;
			/* fix tables */
			for (i=c-1;i<c_nattrs;i++) {
				attributes[i]=attributes[i+1];
				free(attrtypes[i]);
				attrtypes[i]=(char *)malloc(strlen(attrtypes[i+1])+1);
				strcpy(attrtypes[i],attrtypes[i+1]);
			}
			free(attrtypes[c_nattrs-1]);
			c_nattrs--;
			menu_del_item(attrmenu,0,c);
		}
		c=ATTRIBUTE;
	   break;
	   case TOGGLESTAT:
		up_gwin=up_gwin?0:1;
		c=ATTRIBUTE;
	   break;
	   /* color the menu with attributes */
	   case KEY_UP:
		menu_set_default(attrmenu,"-1",0);
		c=ATTRIBUTE;
	   break;
	   case KEY_DOWN:
		menu_set_default(attrmenu,"+1",0);
		c=ATTRIBUTE;
	   break;
	   case KEY_PPAGE:
		/* change forground */
		if (++foreground == 0x10) foreground = 0;
		if (foreground == 0 && background == 0) foreground = 1;
		attributes[j-1] = (background << 4) + foreground;
		c=ATTRIBUTE;
		c_code=j-1;
	   break;
	   case KEY_NPAGE:
		/* change background */
		background = (background+1) & 0xF;
		attributes[j-1] = (background << 4) + foreground;
		c=ATTRIBUTE;
		c_code=j-1;
	   break;
	}
break;
case FILES:
	/* bring up screen menu */
	c=menu(filemenu,y_place(filemenu),x_place(filemenu,1,0),space1,&spc,&j,1);
	switch (spc) {
	   case KEY_LEFT: menu_set_default(genmenu,0,EXIT); break;
	   case KEY_RIGHT: menu_set_default(genmenu,0,ATTRIBUTE); break;
	   case KEY_DOWN:
			menu_set_default(filemenu,"+1",0);
			if (!c) c_screen=-1;
			else {
			    if (c==fcnt) c=0;
			    c_screen=c;
			}
			c=FILES;
	   break;
	   case KEY_UP:
			menu_set_default(filemenu,"-1",0);
			if (!c) c_screen=-1;
			else {
			    if (c==1) c=fcnt+1;
			    c_screen=c-2;
			}
			c=FILES;
	   break;
	   case DEL:
			/* delete a .scr file */
			if (fcnt) {
			    popup_str(LINES/2,COLS/4,"Delete this filename from menu?",ASK_WIN,1,3,ans);
			    if (strchr(strlwr(ans),(int)'y'))
			    if (c-1==c_loaded) c_loaded=-1;
			    if (c!=fcnt) {
				for (i=c;i<=fcnt;i++)
				    menu_set_code(filemenu,fnptrs[i],i);
				i=ABS(cfna-fnptrs[c-1]);
				cfna-=strlen(fnptrs[c-1])+1;
				memmove(fnptrs[c-1],fnptrs[c],i);
				c_screen=c-1;
			    } else
				if (fcnt==1) c_screen=-1;
				else c_screen=c-2;
			    menu_del_item(filemenu,space1,0);
			    cfnptrs--;
			    fcnt--;
			}
			else ERROR("No filenames to delete");
			c=FILES;
	   break;
	   case INS:
			/* creating a new screen */
			popup_str(LINES/2,0,"Enter filename:",ASK_WIN,1,60,ans);
			if (strlen(ans)) {
				i=getfiles(ans);
				if (!i && !strchr(ans,(int)'*') && !strchr(ans,(int)'?')) {
				/* add item to menu */
					stcgfn(node,ans);
					menu_add_item(filemenu,node,10000,0);
					strcpy(cfna,ans);
					*cfnptrs=cfna; cfna+=strlen(ans)+1;
					fcnt++; cfnptrs++;
				}
				/* switch to new screen ? */
				if (c_screen=-1&&fcnt) c_screen=0;
			}
			c=FILES;
		break;
	    case CR:
			if (fcnt) {
			    i=k=0;
			    /* check extension on fnptrs[j-1] */
			    if (strstr(fnptrs[j-1],".fld"))      k=1;
			    else if (strstr(fnptrs[j-1],".scr")) k=2;
			    else if (strstr(fnptrs[j-1],".cfg")) k=3;
			    if (k<3) CLEARSCREENS;
			    switch (k) {
				case 0:
				case 1:
				    /* load .fld file */
				    fld_free(fldtop);
				    fldtop=0;
				    i=fld_win_in(extfilename(fnptrs[j-1],"fld",space2),stdcodescr,&fldtop,&numflds,attrtypes,-1,-1,&shdr);
				    if (i) {
					sprintf(space3,"Loaded %s",space2);
					popup_str(LINES/2,0,space3,TELL_WIN,1,-1,0);
				    }
				    if (k==1) break;
				case 2:
				    /* load .scr file */
				    if (!i) {
					i=scr_win_in(extfilename(fnptrs[j-1],"scr",space2), stdcodescr, attrtypes, -1, -1, &shdr);
					if (i) {
					    sprintf(space3,"Loaded %s",space2);
					    popup_str(LINES/2,0,space3,TELL_WIN,1,-1,0);
					}
				    }
				    if (k==2) break;
				case 3:
				    k=init_attrs(extfilename(fnptrs[j-1],"cfg",space2),attributes,MAX_ATTRS);
				    if (k) {
					sprintf(space3,"Loaded %s",space2);
					popup_str(LINES/2,0,space3,TELL_WIN,1,-1,0);
				    }
			    }

			    if (i) {
				c_posy=shdr.pos_y; c_posx=shdr.pos_x;
				c_lines=shdr.rows; c_cols=shdr.cols;
				c_nattrs=shdr.nattrs;
				overwrite(stdcodescr,stdscr);
				c_loaded=j-1;					
				c_y=c_posy; c_x=c_posx;
				/* build dynamic attribute menu */
				menu_end(attrmenu);
				attrmenu=menu_init("CTRL-Attributes",0,MENU_NORMAL,MENU_HILITE,MENU_TITLE,sel_keys,7,esc_keys,5,1,1);
				for (i=0;i<c_nattrs;i++)
				   menu_add_item(attrmenu,attrtypes[i],i+1,0);				
			    }
			    c_screen=j-1;
			}
			c=0;
	   break;
	   case HELPKEY:
			/* info on directory and whether loaded */
			if (fcnt) {
			   if (!stricmp(fnptrs[c-1],space1))
			        sprintf(space2,"%s%c%s",getcwd(node,80),DIRSLASH,space1);
			   else sprintf(space2,"%s",fnptrs[c-1]);
			   popup_str(upgen?c_y+3+c:c_y+1+c,upgen?mx+menu_get_width_items(genmenu)+c+1:c_x+c+1,space2,TELL_WIN,1,-1,0);
			}
			else ERROR("F6 in this menu shows the directory\n of a file, but there are no files");
			c=FILES;
	   break;
	   case TOGGLESTAT:
			up_gwin=up_gwin?0:1;
			c=FILES;
	   break;
	}
break;
case DRAW:
	/* bring up drawing menu */
	c=menu(drawmenu,y_place(drawmenu),x_place(drawmenu,3,0),0,&spc,0,1);
	switch (spc) {
	   case KEY_LEFT: menu_set_default(genmenu,0,ATTRIBUTE); break;
	   case KEY_RIGHT: menu_set_default(genmenu,0,EXIT); break;
	   case HELPKEY: help_menu(drawmenu,c,c_draw); c=DRAW; break;
	   case TOGGLESTAT: up_gwin=up_gwin?0:1; c=DRAW; break;
	}
	updraw=(c&&c!=DRAW)?1:0;
break;

/* exit menu item selections */
case VERS:
	popup_str(LINES/2,COLS/4,rcsid,TELL_WIN,1,-1,0);
	c=EXIT;
break;
case ASCII:
	if (!
#ifdef DOS
	popup_file(1,1,"ascii.dos",TELL_WIN,1,22,74)
#else
	popup_file(1,1,"ascii.qnx",TELL_WIN,1,22,74)
#endif
	) ERROR("Can't find ascii codes file");
	c=EXIT;
break;
case EXIT:
	/* bring up exit menu */
	c=menu(exitmenu,y_place(exitmenu),x_place(exitmenu,4,0),0,&spc,0,1);
	switch (spc) {
	   case KEY_LEFT: menu_set_default(genmenu,0,DRAW); break;
	   case KEY_RIGHT: menu_set_default(genmenu,0,FILES); break;
	   case HELPKEY: help_menu(exitmenu,c,c_draw); c=EXIT; break;
	   case TOGGLESTAT: up_gwin=up_gwin?0:1; c=EXIT; break;
	}
	if (c==KEYS) {
		help_menu(exitmenu,c,c_draw);
		c=EXIT;
	}
	if (c==HELPKEY) {
		help_menu(-1,0,c_draw);
		c=EXIT;
	}
	upexit=(c&&c!=EXIT&&c!=TOGGLESTAT)?1:0;
	if (c==ESC) /* quit prog */
		upgen=updraw=upexit=0;	
	if (c!=ESC) break;
case ESC:
	if (!upgen&&!updraw&&!upexit) {
	    popup_str(LINES/2,COLS/4,"Have you saved all your files,\nand really want to exit?",ASK_WIN,2,3,ans);
	    if (strlen(ans)){
		if (!strchr(strlwr(ans),(int)'y')) c=0;
	    }
	    else c=0;
	}
break;
case SAVE:
	/* bring up save menu */
	c=menu(savemenu,y_place(savemenu),x_place(savemenu,4,7),0,&spc,0,1);
	switch (spc) {
	   case KEY_LEFT: c=EXIT; break;
	   case KEY_RIGHT: break;
	   case HELPKEY: help_menu(savemenu,c,c_draw); c=SAVE; break;
	   case TOGGLESTAT: up_gwin=up_gwin?0:1; c=SAVE; break;
	}
	if (c) switch (c) {
	   case CHOICE1:
		if (c_screen>=0) {
			sprintf(space1,"Save to file %s?",extfilename(fnptrs[c_screen],"scr",space2));
			popup_str(LINES/2,0,space1,ASK_WIN,1,3,ans);
			if (strchr(strlwr(ans),(int)'y'))
			   scr_win_out(extfilename(fnptrs[c_screen],"scr",space2),stdcodescr,attrtypes,c_nattrs,c_posy,c_posx,c_lines,c_cols);
		}
		else ERROR("No filename exists");
		c=0;
		break;
	   case CHOICE2:
		if (c_screen>=0) {
			sprintf(space1,"Save to file %s?",extfilename(fnptrs[c_screen],"cfg",space2));
			popup_str(LINES/2,0,space1,ASK_WIN,1,3,ans);
			if (strchr(strlwr(ans),(int)'y'))
				save_attrs(extfilename(fnptrs[c_screen],"cfg",space2),attributes,c_nattrs);
		}
		else ERROR("No filename exists");
		c=0;
		break;
	   case CHOICE3:  c=0; break;
	   case CHOICE4:
		if (c_screen>=0) {
			sprintf(space1,"Save to file %s?",extfilename(fnptrs[c_screen],"fld",space2));
			popup_str(LINES/2,0,space1,ASK_WIN,1,3,ans);
			if (strchr(strlwr(ans),(int)'y'))
				fld_win_out(extfilename(fnptrs[c_screen],"fld",space2),stdcodescr,fldtop,attrtypes,c_nattrs,c_lines,c_cols,c_posy,c_posx);
		}
		else ERROR("No filename exists");
		c=0;
		break;
	}
	upexit=(c==SAVE)?1:0;
break;
case PREFERENCE:
	c=menu(prefmenu,y_place(prefmenu),x_place(prefmenu,4,7),0,&spc,0,1);
	switch (spc) {
	   case KEY_LEFT: c=EXIT; break;
	   case KEY_RIGHT: c=0; break;
	   case HELPKEY: help_menu(prefmenu,c,c_draw); c=PREFERENCE; break;
	   case TOGGLESTAT: up_gwin=up_gwin?0:1; c=PREFERENCE; break;
	}
	switch (c) {
	   case CHOICE1: if (!init_attrs("color.cfg",pattributes,8))
		        ERROR("Can't find color.cfg");
		c=0; SETMENUATTR;
		break;
	   case CHOICE2: if (!init_attrs("mono.cfg",pattributes,8))
			ERROR("Can't find mono.cfg");
		c=0; SETMENUATTR;
		break;
	   case CHOICE3: c=ATTRPREFERENCE; break;
	}
	upexit=(c==PREFERENCE)?1:0;
break;
case ATTRPREFERENCE:
	menu_set_attrs(pattrmenu,0,&(pattributes[menu_get_last(pattrmenu)-1]),0);
	c=menu(pattrmenu,y_place(pattrmenu),x_place(pattrmenu,4,0),0,&spc,&j,1);
	foreground = pattributes[j-1] & 0xF;
	background = (pattributes[j-1] >> 4) & 0xF;
	switch(spc) {
		case HELPKEY: help_menu(pattrmenu,0,c_draw); c=ATTRPREFERENCE; break;
		case TOGGLESTAT: up_gwin=up_gwin?0:1; c=ATTRPREFERENCE; break;
		case CR:
		/* write scrdes.cfg */
		save_attrs(extfilename(progname,"cfg",space2),pattributes,8);
		   c=0;
		case ESC: c=0; break;
		case KEY_UP:
  		   menu_set_default(pattrmenu,"-1",0);
		   c=ATTRPREFERENCE;
		break;
		case KEY_DOWN:
		   menu_set_default(pattrmenu,"+1",0);
		   c=ATTRPREFERENCE;
		break;
		case KEY_PPAGE:
		/* change forground */
		   if (++foreground == 0x10) foreground = 0;
		   if (foreground == 0 && background == 0) foreground = 1;
		break;
		case KEY_NPAGE:
		/* change background */
		   background = (background+1) & 0xF;
		break;
	}
	if (spc==KEY_PPAGE||spc==KEY_NPAGE) {
	   pattributes[j-1] = (background << 4) + foreground;
	   c=ATTRPREFERENCE;
	   if (j==1||j==2||j==3) SETMENUATTR;
	}
break;
case OPTION:
	/* bring up mode menu depending on c_draw */
	switch (c_draw) {
	case SCREEN:
	case FILL:
	case LINE: i=linemenu; break;
	case FIELD: i=fieldmenu; break;
	case TEXT: i=textmenu; break;
	case BUFFER: i=bufmenu; break;
	case ERASE: i=erasemenu; break;
	}
	c=menu(i,y_place(i),x_place(i,3,7),0,&spc,0,1);
	switch (spc) {
	   case HELPKEY: help_menu(i,c,c_draw); c=OPTION; break;
	   case TOGGLESTAT: up_gwin=up_gwin?0:1; c=OPTION; break;
	   case KEY_LEFT:
	   case KEY_RIGHT: c=DRAW; break;
	}
	updraw=(c==OPTION)?1:0;
break;
/* line type and text justification keys */
case THREE:
	if (mode==-(c-ONE+1)&&!dblmode) dblmode=1;
	else dblmode=0;
	if (c_draw==LINE||c_draw==FILL||c_draw==SCREEN&&region==TEST_REGION) {
	    popup_str(LINES/2,COLS/4,"Enter character enclosed in quotes\nor decimal ascii value:",ASK_WIN,1,3,ans);
	    if (strlen(ans)) {
		if (ans[0]=='\''||ans[0]=='\"')
		   mode=(int)ans[1];
		else mode=atoi(ans);
	    }
	}
	else mode=OPTION3;
	c=0;
break;
case FOUR:
	mode=OPTION4;
	if (c_draw == TEXT && region == TEST_REGION)
		if (c_screen < 0 || (txtfp=fopen(fnptrs[c_screen],"r"))==0) {
		    ERROR("Can't find file");
		    mode=OPTION1;
		}
	c=0;
break;
case TWO:
case ONE:
	if (mode==-(c-ONE+1)&&!dblmode) dblmode=1;
	else {
		mode=-(c-ONE+1);
		dblmode=0;
	}
	c=0;
break;
/* define region keys */
case HOME:
	r_h_x=r_e_x=c_x;
	r_h_y=r_e_y=c_y;
	region=START_REGION;
	c=0;
break;
case END:
	if (region==START_REGION) {
		region=TEST_REGION;
		r_e_x=c_x;
		r_e_y=c_y;
		c_x=r_h_x;
		c_y=r_h_y;
		if (c_draw==TEXT) {
		    setup_text(r_h_x,r_h_y,r_e_x,r_e_y);
		    if (mode==OPTION4)
			if (c_screen < 0 || (txtfp=fopen(fnptrs[c_screen],"r"))==0) {
				ERROR("Can't find file");
				mode=OPTION1;
			}
		}
	}
	c=0;
break;
/* drawing element keys */
case SCREEN: c_lines=LINES; c_cols=COLS; c_posx=0; c_posy=0;
case TEXT: if (region==TEST_REGION && c!=SCREEN)
		setup_text(r_h_x,r_h_y,r_e_x,r_e_y);
case FIELD:
case LINE:
case FILL:
case BUFFER:
case ERASE:
	c_draw=c;
	if (updraw) c=OPTION;
	else c=0;
	mode=OPTION1;
break;
/* accept region */
case INS:
	if (region==TEST_REGION) {
		c_x=r_h_x;
		c_y=r_h_y;
		if (c_draw==FIELD) {
		    switch (mode) {
			case OPTION1: /* create */
			    /* get field name */
			    popup_str(LINES/2,0,"Enter field text:",ASK_WIN,1,60,fldtxt);
			    /* get field number */
			    popup_str(LINES/2,COLS/4,"Enter field number:",ASK_WIN,1,3,ans);
			    fldnum=atoi(ans);
			    numflds++;
			    if (!fldnum) fldnum=numflds;
			    sprintf(space2,"Field number set to %d",fldnum);
			    popup_str(LINES/2,COLS/4,space2,TELL_WIN,1,-1,0);
			    break;
			case OPTION2: /* delete */
			    /* get a field that the region is in */
			    i=fld_num(fldtop,r_h_y, r_h_x, r_e_y, r_e_x,&fldptr);
			    if (i!=-1) do {
				sprintf(space1,"Delete field (%d:%s)?",i,fldptr->txt);
				popup_str(LINES/2,0,space1,ASK_WIN,1,3,ans);
				fldptr=fldptr->fldnext;
				if (strchr(strlwr(ans),(int)'y')) {
				    /* delete it */
				    if ((fld_del(&fldtop,i))>=0) {
					sprintf(space1,"Field %d deleted",i);
					popup_str(LINES/2,COLS/4,space1,TELL_WIN,1,-1,0);
				    }
				    else ERROR("error deleting field");
				}
			    }
			    while ( (i=fld_num(fldptr,r_h_y, r_h_x, r_e_y, r_e_x,&fldptr)) !=-1);
			    if (i==-1) {
				popup_str(LINES/2,COLS/4,"No more fields here",TELL_WIN,1,-1,0);
				c=0;
			    }
			break;
			case OPTION3: /* move */;
			    i=fld_num(fldtop,r_h_y, r_h_x, r_e_y, r_e_x,&fldptr);
			    if (i!=-1) do {
				sprintf(space1,"Move field (%d:%s)?",i,fldptr->txt);
				popup_str(LINES/2,0,space1,ASK_WIN,1,3,ans);
				if (strchr(strlwr(ans),(int)'y')) {
				    putfield=1;
				    break;
				}
				fldptr=fldptr->fldnext;
			    }
			    while ( (i=fld_num(fldptr,r_h_y, r_h_x, r_e_y, r_e_x,&fldptr)) !=-1);
			    if (i==-1) {
				popup_str(LINES/2,COLS/4,"No more fields here",TELL_WIN,1,-1,0);
			    }
			    c=0;
			    break;
			case OPTION4: /* info */
			    /* get a field that the region is in */
			    i=fld_num(fldtop,r_h_y, r_h_x, r_e_y, r_e_x,&fldptr);
			    if (i!=-1) do {
				sprintf(space1,"NUMBER:\t%d\nTEXT:\t%s\nLINE:\t%d\nCOLUMN:\t%d\nWIDTH:\t%d\nLENGTH:\t%d\nATTRIBUTE:\t%s",
					fldptr->num,fldptr->txt,fldptr->line,fldptr->col,fldptr->width,fldptr->length,attrtypes[fldptr->attrcode]);
				popup_str(LINES/2,COLS/4,space1,TELL_WIN,1,-1,0);
				fldptr=fldptr->fldnext;
			    }
			    while ( (i=fld_num(fldptr,r_h_y, r_h_x, r_e_y, r_e_x,&fldptr)) !=-1);
			    if (i==-1) {
				popup_str(LINES/2,COLS/4,"No more fields here",TELL_WIN,1,-1,0);
				c=0;
			    }
		    }
		}
	}
	if (c_draw==FIELD && mode==OPTION3 && region==NO_REGION && putfield==1)
	    putfield=2;
	else if (c_draw==BUFFER) {
	    if (region==TEST_REGION)
		putbuffer=0;
	}
	else if (region!=TEST_REGION) c=0;
	region=NO_REGION;
	break;
/* kill region */
case DEL:
	if (region!=NO_REGION) {
		region=NO_REGION;
		c_x=r_h_x;
		c_y=r_h_y;
	}
	c=0;
break;
/* help and non-accepted keys */
/* first time through this loop */
case FIRST: c=GENERAL; break;
case HELPKEY:
	switch (region) {
	case NO_REGION: sprintf(space1,"No region exists.\nStart defining a region with the HOME key.\n"); break;
	case START_REGION: sprintf(space1,"You have started to define a region.\nUse the arrow keys to form the region, then press END.\n"); break;
	case TEST_REGION: sprintf(space1,"You have defined a region.\nChoose a drawing element, option and attribute type.\nThen press INS to commit or DEL to delete.\n");
	}
	if (bufwin) strcat(space1,"A buffer exists.\n");
	else strcat(space1,"No buffer exists.\n");
	if (fldtop) strcat(space1,"Fields exist.\n");
	else strcat(space1,"No Fields exist.\n");
	strcat(space1,"More help is found in the Exit Menu (Ctrl E).\n");
	popup_str(2,8,space1,HELP_WIN,2,-1,0);
c=0;
break;
/* toggle on or off the general window */
case TOGGLESTAT:
	up_gwin=up_gwin?0:1;
	c=0;
break;
default:
	if (region==TEST_REGION && c_draw==TEXT) {
		c_txt[0]=(char)c;
		c_txt[1]='\0';
	}
	else beep();
	c=0;
} /* switch */


/* UPDATE SCREEN */
move(c_y,c_x);
if (!WrtAttrTable(stdscr,stdcodescr,attributes,c_posy,c_posx,c_lines,c_cols)) {
    ERROR("Out of memory");
    break;
}
wnoutrefresh(stdscr);
overwrite(stdcodescr,curcodescr);

/* accept a test region */
if (c==INS) {
	handle_region(c_draw, mode, c_code, r_h_x, r_h_y, r_e_x, r_e_y);
	overwrite(curscr,stdscr);
	overwrite(curcodescr,stdcodescr);
}

/* general window */
if (up_gwin) {
	wholemsg[0]='\0';
	strcpy(attrmsg,AMSG);
	for (i=0;i<11&&attrtypes[c_code][i]!='\0';i++)
	   attrmsg[i+12]=attrtypes[c_code][i];
	strcpy(screenmsg,SMSG);
	if (c_loaded>-1) {
		stcgfn(node,fnptrs[c_loaded]);
		for (i=0;i<11&&node[i]!='\0';i++)
		screenmsg[i+12]=node[i];
	}
	strcpy(filemsg,FMSG);
	if (c_screen>-1) {
		stcgfn(node,fnptrs[c_screen]);
		for (i=0;i<11&&node[i]!='\0';i++)
		filemsg[i+12]=node[i];
	}
	switch(c_draw) {
		case LINE: strcpy(elmsg+12,drawmsgs[0]);
		strcpy(optionmsg+12,linemsgs[mode>0?2:ABS(mode)-1]);
		break;
		case TEXT: strcpy(elmsg+12,drawmsgs[1]);
		strcpy(optionmsg+12,textmsgs[ABS(mode)-1]);
		break;
		case FILL: strcpy(elmsg+12,drawmsgs[2]);
		strcpy(optionmsg+12,linemsgs[mode>0?2:ABS(mode)-1]);
		break;
		case ERASE: strcpy(elmsg+12,drawmsgs[3]);
		strcpy(optionmsg+12,erasemsgs[ABS(mode)-1]);
		break;
		case BUFFER: strcpy(elmsg+12,drawmsgs[4]);
		strcpy(optionmsg+12,buffermsgs[ABS(mode)-1]);
		break;
		case SCREEN: strcpy(elmsg+12,drawmsgs[5]);
		strcpy(optionmsg+12,linemsgs[mode>0?2:ABS(mode)-1]);
		break;
		case FIELD: strcpy(elmsg+12,drawmsgs[6]);
		strcpy(optionmsg+12,fieldmsgs[ABS(mode)-1]);
		break;
	}
	strcat(wholemsg,attrmsg);
	strcat(wholemsg,screenmsg);
	strcat(wholemsg,filemsg);
	strcat(wholemsg,elmsg);
	strcat(wholemsg,optionmsg);
	if (px==c_x&&c_y>=py&&c_y<py+7)
	   px+=(px+26<COLS)?1:-26;
	if (px+24+1==c_x&&c_y>=py&&c_y<py+7)
	   px+=(px-1>0)?-1:26;
	if (py==c_y&&c_x>=px&&c_x<px+24+2)
	   py+=(py+7<LINES)?1:-7;
	if (py+5+1==c_y&&c_x>=px&&c_x<px+24+2)
	   py+=(py-1>0)?-1:7;
	gwin=popup_win(py,px,wholemsg,STATUS_WIN,2,0);
	mvWrtNAttr(gwin,attributes[c_code],24,1,1);
	wrefresh(gwin);
	delwin(gwin);
/*	popup_str(py,px,wholemsg,STATUS_WIN,2,0,0);*/
/*	mvWrtNAttr(curscr,attributes[c_code],24,py+1,px+1);*/

}
wmove(curscr,c_y,c_x);


/* menus */
if (c!=INS) {
  if (region==TEST_REGION || c_draw==BUFFER || (c_draw==FIELD && mode==OPTION3))
	handle_region(c_draw, mode, c_code, r_h_x, r_h_y, r_e_x, r_e_y);
   if (c!=ESC) {
	if (updraw) menu(drawmenu,y_place(drawmenu),x_place(drawmenu,3,0),0,&spc,0,0);
	else if (upexit) menu(exitmenu,y_place(exitmenu),x_place(exitmenu,4,0),0,&spc,0,0);
	if (upgen)
   	   if (c==0) c=GENERAL;
   	   else menu(genmenu,my,mx,0,0,0,0);
   }
}

/* draw region box */
if (region==START_REGION)
    handle_region(c_draw, mode, c_code, r_h_x, r_h_y, c_x, c_y);

doupdate();

/* get next character */
spc=0;
if (c==ESC) break;
if (region!=TEST_REGION) flushinp();
if (c==0||c==INS) c=getch();
} /* while */


/* cleanup */
#ifdef __QNX__
tcsetattr(fileno(stdin),TCSANOW,&option_save);
#endif
if (txt) free(txt);
if (txtline) free(txtline);
menu_end(-1);
attrset(A_NORMAL);
clear();
refresh();
delwin(stdcodescr);
delwin(curcodescr);
if (bufwin) delwin(bufwin);
if (cpbufwin) delwin(cpbufwin);
if (bufcodewin) delwin(bufcodewin);
if (blankbufwin) delwin(blankbufwin);
if (blankcodewin) delwin(blankcodewin);
endwin();
}


int getfiles(char *pat) {
int j,i;
	if ((i=getfnl(pat,cfna,500-(ABS(cfna-fna)),0))>-1) {
		if (strbpl(cfnptrs,50-fcnt,cfna)!=i) {
			ERROR("Too many file names");
			return(0);
		}
		for (j=0;j<i;j++) {
			stcgfn(node,cfnptrs[j]);
			menu_add_item(filemenu,node,10000,0);
			cfna+=strlen(cfna)+1;
		}
	}
	else {
		ERROR("Too many file names or bad file pattern");
		return(0);
	}
	fcnt+=i;
	cfnptrs+=i;
	return(i);
}

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

int x_place(int menu, int mpos, int apos) {
int tx;
int t;
if (upgen && menu!=genmenu)
    tx=mx+(menu_get_width_items(genmenu)*mpos)+1+apos;
else tx=c_x;
if (tx+(t=menu_get_width(menu))>=COLS)
    tx=COLS-t;
return(tx);
}

int y_place(int menu) {
int ty;
int t;
if (upgen && menu!=genmenu) ty=my+2;
else ty=c_y;
if (ty+(t=menu_get_length(menu))>=LINES)
    ty=LINES-t;
return(ty);
}
