/* DISPLAY.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/* This software is the property of Bomem and should be considered and      */
/* treated as proprietary information.  Refer to the "Source Code License   */
/* Agreement"                                                               */
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 DISPLAY.H 26-May-93,16:38:02,`THOMAS' File creation
1:1 DISPLAY.H 26-Nov-93,11:03:24,`MICHEL' New error messages
1:2 DISPLAY.H 15-Apr-94,9:29:24,`JEAN' Added the new TLIB header
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

/* Definition for display sub-system */

#ifndef	BOMEM_DISPLAY
#define	BOMEM_DISPLAY

#ifndef	BOMEM_USEFUL
	#include "USEFUL.H"
#endif

/*
        The relative coordinates are defined as:

              (0,0)
                ------------------------
                |                      | <--- FULL SCREEN
                |                      |
                | (r.x1, r.y1)         |
                |     ----------       |
                |     |        |       |
                |     | Window |       |
                |     |        |       |
                |     ----------       |
                |         (r.x2, r.y2) |
                |                      |
                |                      |
                |                      |
                ------------------------
                                     (1,1)


        This means that you define a region where all outputs will be done
        using coordinate scaled according to world coordinate.  The purpose
        of relative coordinate is to allow device independancy, that is
        to get approximatively the same graphical outputs even if the 
        device resolution changes. 

        The device ooordinate are defined as follow for a VGA screen: 
              (0,0)
                ------------------------
                |                      | <--- FULL SCREEN
                |                      |
                | (d.x1, d.y1)         |
                |     ----------       |
                |     |        |       |
                |     | Window |       |
                |     |        |       |
                |     ----------       |
                |         (d.x2, d.y2) |
                |                      |
                |                      |
                |                      |
                ------------------------
                                   (639, 479)
        For EGA, the lower right position would be (639, 349) 
*/
/* Error message list */
#define ERROR_WINDOWS_2_SMALL	(ERROR_BASE_DISP) /* Text window too small */
#define ERROR_DISP_COORD	(ERROR_BASE_DISP-1) /* Invalid window coordinates */


#define SCREEN_WIDTH 640				/* EGA and VGA screen width */

/* Curve display options */
#define NORMAL	0
#define XOR		1
#define UPDATE  2

/* Color supported */
#define	BO_BLACK		0
#define	BO_BLUE			1
#define	BO_RED			2
#define	BO_GREEN		3
#define	BO_CYAN			4
#define	BO_WHITE		5
#define	BO_YELLOW		6
#define	BO_MAGENTA		7
#define BO_AXECOLOR		8

/*	Graphic cards BIOS mode */
#define BO_EGA_MONO		0x9
#define BO_EGA_COLOR	0x10
#define BO_VGA_MONO		0x11
#define BO_VGA_COLOR	0x12
#define BO_NO_RESET		-1

/*	Character description */
#define CHAR_HEIGHT		9
#define CHAR_BHEIGHT	14				/* Height of the large size char */
#define CHAR_WIDTH		8

/* Dispcurv display state */
#define DISP_HIDDEN			0	/* Do not update the display */
#define DISP_NORMAL			1	/* Update the curves and scales*/
#define DISP_REDRAW			2	/* Redraw everything */

/* Scale labels */
#define DISP_SCALE_LEN		15	/* Number of characters in scale label */

/* Structure for device coordinates */
typedef struct
	{
	short x1, y1;						/* Left upper corner */
	short x2, y2;						/* Right bottom corner */
	} Dcoord;

/* Structure for world or relative coordinates */
typedef struct
	{
	double x1, y1;						/* Left upper corner */
	double x2, y2;						/* Right bottom corner */
	} Wcoord;

typedef struct
		{
		Dcoord d;						/* Current window device coordinates */
		Wcoord w;						/* Current window world coordinates */
		double xmin, xmax, ymin, ymax;	/* Current window world limits */
 		double x_scale;					/* World to device x scaling factor */
		double y_scale;					/* World to device y scaling factor */
		double m_min;					/* Minimum slope */
		double m_max;					/* Maximum slope */
		char x_label[DISP_SCALE_LEN+1];	/* X-axis label for units */
		char y_label[DISP_SCALE_LEN+1];	/* Y-axis label for units */
		} Current_window;

typedef struct
	{
	char valid;							/* TRUE if struct valid */
	struct
		{
		Dcoord d;						/* Frame device coordinates */
		char color;						/* Frame border line color */
		char back_color;				/* Frame background color */
		} frame;
	struct
		{
		char title[81];					/* Title itself */
		char max_length;				/* Title maximum length */
		char color, back_color;			/* Title color and background color */
		} title;
	struct
		{
		char color, back_color;			/* Scale color and background color */
		struct
			{
			short n_digits;				/* Number of digits in scale X */
			short n_ticks;				/* Number of tick marks in scale X */
			short x1, y1, x2, y2;		/* Scale X device coordinates */
			} x;
		struct
			{
			short n_digits;				/* Number of digits in scale Y */
			short n_ticks;				/* Number of tick marks in scale Y */
			short x1, y1, x2, y2;		/* Scale Y device coordinates */
			} y;
		} scale;
	struct
		{
		Dcoord d;						/* Curve region device coordinates */
		char interp;					/* Internal variable */
		char autoscale_x;				/* Internal variable */
		char autoscale_y;				/* Internal variable */
		char color;						/* Current curve color */
		char mode;						/* NORMAL, XOR, UPDATE */
		char fast_upd;					/* Fast update flag: TRUE or FALSE */
		short *old_curve;				/* Fast update buffer, allocated by */
										/* display_curve() and free by */
										/* display_close() */ 
		} curve;
	Current_window cw;					/* Current window information */
	} Display_window;

/* DISPCUR2.C */
short wdisp_curve (Wcoord r, Wcoord world, char *title, short title_col,
				   short scale_col, short curve_col, short back_col,
                   YDATA vector, char x_digit, char y_digit);
/* DISPCURV.C */
short dispcurv_init (Wcoord r, char *title, Display_window **disp,
				     char display_flag, char x_digit, char y_digit);
short dispcurv_disp (Display_window *disp, YDATA vector, Wcoord w,
					 char display_flag);

/* DISPLAY1.C */
Display_window *display_default (void);
Display_window *display_open (Wcoord r, char *title,
							  short x_n_digits, short y_n_digits);
void  display_frame (Display_window *disp);
void  display_scale_hide (Display_window *disp);
void  display_scale_show (Display_window *disp);
short display_set_world (Display_window *disp, Wcoord w);
void display_set_autoscale (Display_window *disp, char autoscale_x,
							char autoscale_y);
void  display_close (Display_window **disp, char clear);

/* D_CURVE.C */
short display_curve (Display_window *disp, YDATA curve);

/* DISPLAY0.C */
short display_init (short video_mode);
#define display_end()	dretgr()

short bo_r_to_d (Wcoord r, Dcoord *d);
short bo_d_to_r (Dcoord d, Wcoord *r);
short bo_x_to_d (Current_window *cw, double x);
short bo_y_to_d (Current_window *cw, double y);

short bo_set_window (Current_window *cw, Wcoord w,  Dcoord d);
void  bo_get_window (Current_window *cw, Wcoord *w, Dcoord *d);

void df__puts(char *s);

/* D_LINE.C */
void bo_line (Current_window *cw, char xor_mode, char color,
			  double x1, double y1, double x2, double y2);

/* D_FILL.C */
void bo_fill (Current_window *cw, char color, char *pattern,
			  double x1, double y1, double x2, double y2);


/* D_XY_DIS.C */
void bo_disp_xy_dis (Current_window *cw, char color, float *x, float *y,
					 short length);

/* D_XY.C */
void bo_disp_xy (Current_window *cw, char color, float *x, float *y,
			     short length);

/* DISPZOOM.C */
void disp_zoom (float HPTR *vin,  double in_x1,  double in_x2,  long  n_in,
			    float *vout, double out_x1, double out_x2, short n_out);

/* DISPTICK.C */
short get_time_tick (double min, double max, double *tick, short max_tick);
short get_tick (double min, double max, double *tick, short max_tick);


/* WINDOW.C */
void window_open (Dcoord *d_win, Dcoord *d_client, char *title);
void window_draw (Dcoord *d_win, Dcoord *d_client, char *title, char title_col,
				  char title_back, char frame_col, char client_col);


/* VGADRV.ASM */
#ifdef __cplusplus
extern "C" {
#endif

void
	dretgr (void),
	df_puts   (char *str),
	db_puts   (char *str),
	dchr_col  (short for_color, short back_color),
	dcur_pos  (short x, short y),
	dcur_off  (void),
	dcur_on   (void);

#ifdef __cplusplus
	}
#endif

#ifdef __WATCOMC__
	#pragma aux (ASM_RTN) dretgr;
	#pragma aux (ASM_RTN) df_puts;
	#pragma aux (ASM_RTN) db_puts;
	#pragma aux (ASM_RTN) dchr_col;
	#pragma aux (ASM_RTN) dcur_pos;
	#pragma aux (ASM_RTN) dcur_off;
	#pragma aux (ASM_RTN) dcur_on;
#endif

/* Macro for compatibility with old function names */
#define wdisp_init(a)		display_init(a)
#define wdisp_close()		display_end()


#endif
