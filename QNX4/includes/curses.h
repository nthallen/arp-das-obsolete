/* LATTICE CURSES V Screen Manager Library.			*/
/* Copyright (C) 1985,1989  Lattice, Incorporated.		*/
/* LATTICE is a registered trademark of Lattice, Incorporated.	*/

/* Include file  "curses.h".  Externals and macro definitions.	*/

#ifndef _CURSES_H_INCLUDED
#define _CURSES_H_INCLUDED

#include <stdio.h>
#include <sys/dev.h>
#include <sys/qnxterm.h>

/* Miscellaneous #define's					*/

#define		OK	(1)
#define		ERR	(0)

#define		reg	register
#define		bool	char
#undef TRUE
#define		TRUE	(1)
#undef FALSE
#define		FALSE	(0)


#define		INPUT_BUFFER_SIZE	120

/* Window flags							*/
#define	WIN_PAD			0x01
#define	WIN_SUBWIN		0x02
#define	WIN_SCROLL		0x04
#define	WIN_KEYPAD		0x08
#define	WIN_NODELAY		0x10

/* Window structure declaration					*/

#define		WINDOW	struct _win_st

struct	_win_st
	      {
		short	_begy;		/* y home coord of this window	    */
		short	_begx;		/* x home coord of this window	    */
		short	_maxy;		/* number of rows in window	    */
		short	_maxx;		/* number of cols in window	    */
		short	_cury;		/* cursor y coord (rel to _begy)    */
		short	_curx;		/* cursor x coord (rel to _begx)    */
		short	_bufwidth;	/* multiplier for rows in buffer    */
		char	_win_flags;	/* pad,subwin,scroll,keypad,nodelay */
		short	_top_scroll;	/* top of buffer scrolling region   */
		short	_bot_scroll;	/* last scroll line		    */
		char	char_attr;	/* current attributes for new chars */
		char	fill_attr;	/* attribute for clearing lines	    */
		char *	scr_image;	/* chars, attributes on screen	    */
	      };

/* Externals							*/

extern	int	LINES;		/* Number of lines on the terminal */
extern	int	COLS;		/* Nuber of columns on the terminal */

extern	char	normal_attr, n_attr;	/* Normal character attribute */
extern int spc;
extern unsigned short *_q_table;
extern unsigned int _tty;
 
extern	char	ttytype[];	/* Name of terminal type, monitor, size */
extern	bool	_echoit;	/* Echo input to window if true */
extern	bool	NONL;		/* map CR/LF to newlines in waddch() if FALSE*/
extern	bool	_raw_mode;	/* Input a char at a time */

extern	int	_ibuf_start;	/* Index of start of input buffer */
extern	int	_ibuf_end;	/* Index of end of input buffer */
extern	int	_input_buffer[];/* Input buffer */

extern	int	beep_frequency;	/* Error beep - default 720	*/

/* To override box command parameters and always use a single line   */
/* graphics box, set these variables to VERT_SINGLE and HORIZ_SINGLE */
extern	char	box_vertical;	/* Vertical   line for box (if != 0) */
extern	char	box_horizontal;	/* Horizontal line for box (if != 0) */

extern	char *	Def_term;	/* Not used - for compatibility */
extern	bool	My_term;	/* Not used - for compatibility */

extern	WINDOW *	curscr;	/* Current screen image structure pointer */
extern	WINDOW *	stdscr;	/* Standard screen image structure pointer */
extern	WINDOW *	absscr;	/* private absolute screen */

/* Attributes							*/

#define	A_NORMAL	0x0000
#define	A_BLANK		0x0100
#define	A_REVERSE	0x0200
#define	A_BOLD		0x0400
#define	A_UNDERLINE	0x0800
#define	A_BLINK		0x1000
#define A_STANDOUT	(A_REVERSE | A_BOLD)

#define DOSblink 0x80
#define DOSintense 0x08
#define DOSfgcolors 0x07
#define DOSbgcolors 0x70
#define DOSbold 0x08
#define DOSunderline 0x01
#define DOSreverse_video 0x70
#define DOSnormal DOSfgcolors

/* the following aren't used, QNX attributes, but good reference for other programs */
/* #define NORMAL          0x0700
#define BLINK			0x0001
#define BOLD			0x0002
#define INVERSE			0x0004
#define UNDERLINE		0x0008
#define STANDOUT		( INVERSE | BOLD )
*/

/* Keypad Function definitions:						*/
/* When in keypad() mode, function keys will return KEY_FUNCTION	*/
/* ORed with the scan code.  These values are defined below.		*/

#define	KEY_FUNCTION	0x0100

#define	KEY_F1		K_F1		/* Function Keys */
#define	KEY_F2		K_F2
#define	KEY_F3		K_F3
#define	KEY_F4		K_F4
#define	KEY_F5		K_F5
#define	KEY_F6		K_F6
#define	KEY_F7		K_F7
#define	KEY_F8		K_F8
#define	KEY_F9		K_F9
#define	KEY_F10		K_F10

#define	KEY_SF1		K_SHF_F1	/* Shifted Function Keys */
#define	KEY_SF2		K_SHF_F2
#define	KEY_SF3		K_SHF_F3
#define	KEY_SF4		K_SHF_F4
#define	KEY_SF5		K_SHF_F5
#define	KEY_SF6		K_SHF_F6
#define	KEY_SF7		K_SHF_F7
#define	KEY_SF8		K_SHF_F8
#define	KEY_SF9		K_SHF_F9
#define	KEY_SF10	K_SHF_F10

#define	KEY_CF1		K_CTL_F1	/* Control Function Keys */
#define	KEY_CF2		K_CTL_F2
#define	KEY_CF3		K_CTL_F3
#define	KEY_CF4		K_CTL_F4
#define	KEY_CF5		K_CTL_F5
#define	KEY_CF6		K_CTL_F6
#define	KEY_CF7		K_CTL_F7
#define	KEY_CF8		K_CTL_F8
#define	KEY_CF9		K_CTL_F9
#define	KEY_CF10	K_CTL_F10

#define	KEY_AF1		K_ALT_F1	/* Alternate Function Keys */
#define	KEY_AF2		K_ALT_F2
#define	KEY_AF3		K_ALT_F3
#define	KEY_AF4		K_ALT_F4
#define	KEY_AF5		K_ALT_F5
#define	KEY_AF6		K_ALT_F6
#define	KEY_AF7		K_ALT_F7
#define	KEY_AF8		K_ALT_F8
#define	KEY_AF9		K_ALT_F9
#define	KEY_AF10	K_ALT_F10

#define	KEY_DOWN	K_DOWN
#define	KEY_UP		K_UP
#define	KEY_LEFT	K_LEFT
#define	KEY_RIGHT	K_RIGHT
#define	KEY_HOME	K_HOME
#define	KEY_END		K_END
#define	KEY_INS		K_INSERT
#define	KEY_IC		KEY_INS
#define	KEY_DEL		K_DELETE
#define	KEY_DC		KEY_DEL
#define	KEY_PGUP	K_PGUP
#define	KEY_PPAGE	KEY_PGUP
#define	KEY_PGDN	K_PGDN
#define	KEY_NPAGE	KEY_PGDN


/* Graphics characters for boxes */

#define	HORIZ_SINGLE	((char) 196)
#define	HORIZ_DOUBLE	((char) 205)
#define	HORIZ_HALF	((char) 220)
#define	VERT_SINGLE	((char) 179)
#define	VERT_DOUBLE	((char) 186)
#define	VERT_HALF	((char) 221)
#define	SHADE_25	((char) 176)
#define	SHADE_50	((char) 177)
#define	SHADE_75	((char) 178)
#define	BLOCK_SOLID	((char) 219)


/* Macro definitions for standard screen pseudo functions	*/

#define		addch(char)		waddch(stdscr, char)
#define		getch()			wgetch(stdscr)
#define		addstr(str)		waddstr(stdscr, str)
#define		getstr(str)		wgetstr(stdscr, str)
#define		move(y, x)		wmove(stdscr, y, x)
#define		clear()			wclear(stdscr)
#define		erase()			wclear(stdscr)
#define		werase(win)		wclear(win)
#define		clrtobot()		wclrtobot(stdscr)
#define		clrtoeol()		wclrtoeol(stdscr)
#define		insertln()		winsertln(stdscr)
#define		deleteln()		wdeleteln(stdscr)
#define		refresh()		wrefresh(stdscr)
#define		inch()			winch(stdscr)
#define		insch(char)		winsch(stdscr, char)
#define		delch()			wdelch(stdscr)
#define		attron(attr)		wattron(stdscr,attr)
#define		attroff(attr)		wattroff(stdscr,attr)
#define		attrset(attr)		wattrset(stdscr,attr)

#define		standout()		attron(A_STANDOUT)
#define		standend()		attroff(A_STANDOUT)
#define		wstandout(win)		wattron(win,A_STANDOUT)
#define		wstandend(win)		wattroff(win,A_STANDOUT)

#define		raw()			(_raw_mode=TRUE)
#define		noraw()			(_raw_mode=FALSE)
#define		cbreak()		(_raw_mode=TRUE)
#define		nocbreak()		(_raw_mode=FALSE)

/* move (mv and mvw) pseudo functions				*/

#define	mvwaddch(win,y,x,ch)	(wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define	mvwgetch(win,y,x)	(wmove(win,y,x)==ERR?ERR:wgetch(win))
#define	mvwaddstr(win,y,x,str)	(wmove(win,y,x)==ERR?ERR:waddstr(win,str))
#define mvwgetstr(win,y,x,str)	(wmove(win,y,x)==ERR?ERR:wgetstr(win,str))
#define	mvwinch(win,y,x)	(wmove(win,y,x)==ERR?ERR:winch(win))
#define	mvwdelch(win,y,x)	(wmove(win,y,x)==ERR?ERR:wdelch(win))
#define	mvwinsch(win,y,x,c)	(wmove(win,y,x)==ERR?ERR:winsch(win,c))

#define	mvaddch(y, x, ch)	mvwaddch(stdscr, y, x, ch)
#define	mvgetch(y, x)		mvwgetch(stdscr, y, x)
#define	mvaddstr(y, x, str)	mvwaddstr(stdscr, y, x, str)
#define	mvgetstr(y, x,str)	mvwgetstr(stdscr, y, x,str)
#define	mvinch(y, x)		mvwinch(stdscr, y, x)
#define	mvdelch(y, x)		mvwdelch(stdscr, y, x)
#define	mvinsch(y, x, c)	mvwinsch(stdscr, y, x, c)

/* Other pseudo functions					*/

#define	baudrate()		(19200)
#define	erasechar()		('\b')
#define	has_ic()		(TRUE)
#define	has_il()		(TRUE)
#define	idlok(win,bf)		(bf)
#define	killchar()		(0)
#define	flagon(win,flag)	(win->_win_flags|=flag)
#define flagoff(win,flag)	(win->_win_flags&=~flag)
#define flagset(win,bf,flag)	(bf?flagon(win,flag):flagoff(win,flag))
#define	scrollok(win,bf)	flagset(win,bf,WIN_SCROLL)
#define	keypad(win,bf)		flagset(win,bf,WIN_KEYPAD)
#define	nodelay(win,bf)		flagset(win,bf,WIN_NODELAY)
#define savetty()   {_tty=dev_mode(fileno(stdin),0,0);_raw_mode=FALSE;_echoit=TRUE;}
#define resetty()   {dev_mode(fileno(stdin),_tty,_DEV_MODES);_raw_mode=FALSE;_echoit=TRUE;}
#define	echo()			(_echoit = TRUE)
#define	noecho()		(_echoit = FALSE)
#define	nl()			(NONL = FALSE)
#define	nonl()			(NONL = TRUE)
#define	longname()		(ttytype)
#define	getyx(win,y,x)		y = win->_cury, x = win->_curx
#define	wsetscrreg(win,top,bot)	win->_top_scroll=top, win->_bot_scroll=bot
#define	setscrreg(top,bot)	wsetscrreg(stdscr,top,bot)

/* Null functions - provided for compatibility			*/

#define	clearok(win,bf)		(bf)
#define	leaveok(win,bf)		(bf)
#define	flushok(win,bf)		(bf)
#define	crmode()		(OK)
#define	nocrmode()		(OK)
#define	touchwin(win)		(OK)
#define	gettmode()		(OK)
#define	setterm(name)		(OK)
#define	tstp()			(OK)

/* Function declarations					*/

extern void	beep	 (void);
extern int	box	 (WINDOW *, char, char);
extern void	delay_output( int );
extern int	delwin	 (WINDOW *);
extern int	doupdate (void);
extern int	endwin	 (void);
extern void	flash	 (void);
extern void	flushinp (void);
extern int	initscr	 (void);
extern int	mvcur	 (int, int, int, int);
extern int	mvprintw (int, int, char *, ...);
extern int	mvscanw  (int, int, char *, ...);
extern int	mvwin	 (WINDOW *, int, int);
extern int	mvwprintw(WINDOW *, int, int, char *, ...);
extern int	mvwscanw (WINDOW *, int, int, char *, ...);
extern WINDOW *	newpad	 (int, int);
extern WINDOW *	newwin	 (int, int, int, int);
extern int	overlay	 (WINDOW *, WINDOW *);
extern int	overwrite(WINDOW *, WINDOW *);
extern int	pnoutrefresh(WINDOW *,int,int,int,int,int,int);
extern int	prefresh(WINDOW*,int,int,int,int,int,int);
extern int	printw	 (char *, ...);
extern int	scanw	 (char *, ...);
extern int	scroll	 (WINDOW *);
extern WINDOW *	subwin	 (WINDOW *, int, int, int, int);
extern char *	unctrl	 (int);
extern int	waddch	 (WINDOW *, int);
extern int	waddstr	 (WINDOW *, char *);
extern void	wattroff (WINDOW *, int );
extern void	wattron	 (WINDOW *, int );
extern void	wattrset (WINDOW *, int );
extern int	wclear	 (WINDOW *);
extern int	wclrtobot(WINDOW *);
extern int	wclrtoeol(WINDOW *);
extern int	wdelch	 (WINDOW *);
extern int	wdeleteln(WINDOW *);
extern int	wgetch	 (WINDOW *);
extern int	wgetstr	 (WINDOW *, char *);
extern int	winch	 (WINDOW *);
extern int	winsch	 (WINDOW *, int);
extern int	winsertln(WINDOW *);
extern int	wmove	 (WINDOW *, int, int);
extern int	wnoutrefresh(WINDOW *);
extern int	wprintw	 (WINDOW *, char *, ...);
extern int	wrefresh (WINDOW *);
extern int	wscanw	 (WINDOW *, char *, ...);

#endif
