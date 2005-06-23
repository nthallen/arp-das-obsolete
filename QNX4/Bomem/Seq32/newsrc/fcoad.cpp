/* FCOAD.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						    SOURCE CODE										*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 FCOAD.CPP 21-Dec-94,11:06:56,`CLAUDE' File creation
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

/*#$%!*************************************************************************
					 COPYRIGHT (C) BOMEM INC, 1994

Name:   FCOAD
File:   FCOAD.CPP
Author: Claude Lafond
Date:   Dec 1, 1994

Description
		Acquisition program for MR100 and MR200

		Requires:  coamb100.bin, coamb200.bin, sin_cos.bin, fcoad.cfg
 #$%!........................................................................*/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

typedef unsigned short word;

#include "useful.h"
#include "vector.h"
#include "bo_timer.h"
#include "file_gal.h"
#include "setup.h"
#include "bo_wind.h"
#include "seq32_pc.h"
#include "stat.h"

//----------------------------------------------------------------------------
//	Command line default parameters
//----------------------------------------------------------------------------
char vga   	 	  	   = TRUE;			   	// TRUE if VGA card else EGA
Dcoord scr		 	   = {0, 0, 639, 479};  // Screen device coordiantes (VGA)
short mode			   = BO_VGA_COLOR;	   	// Display mode
const long max_int_len = 32768l;			// Maximum interferogram length
short file_ctr 		   = 0;					// File counter startup point
//----------------------------------------------------------------------------
//	Display screen default colors
//----------------------------------------------------------------------------
short border_color 	   = BO_WHITE;
short title_color	   = BO_WHITE;
short title_back_color = BO_BLUE;

short scale_color	   = BO_WHITE;
short scale_back_color = BO_BLUE;

short cur_color		   = BO_YELLOW;

const short COLOR_A	   = BO_YELLOW;
const short COLOR_B	   = BO_GREEN;
//----------------------------------------------------------------------------
//	Configuration file stuff for read_setup
//----------------------------------------------------------------------------
short disp_dir	 = FALSE;		 		// Display direction normal

char  prefix[9]	 = "BOMEM";
short resolution = 16;
long  n_seq		 = 10;
long  n_scans	 = 10;;
float coad_delay = 0;
short data_type	 = 0;

short instrument = MR100;
short channel_a  = FALSE;
short over_a	 = FALSE;
short gain_a	 = 1;
float delay_a	 = 3.1;
float sna		 = 1500;
float sxa		 = 6000;
short channel_b	 = FALSE;
short over_b	 = FALSE;
short gain_b	 = 1;
float delay_b	 = 3.1;
float snb		 = 1500;
float sxb		 = 6000;

short irq		 = 3;
short seq32_base = 0x300;

float adc_max	 = 5;
double laser	 = 15799.7;

short true (struct commands *cmd_table, short keyw_index, char *line,
			short parm_index)
{

	line;
	parm_index;
	*(short *)cmd_table[keyw_index].ptr = TRUE;
	return NO_ERROR;
}

short t_ins (struct commands *cmd_table, short keyw_index, char *line,
			 short parm_index)
{
short n;

	line;
	parm_index;
	n = *(short *)cmd_table[keyw_index].ptr;
	if (n == 1  ||  n == 5) return NO_ERROR;
	return ERROR;
}
short t_res (struct commands *cmd_table, short keyw_index, char *line,
			 short parm_index)
{
short i, n;

	line;
	parm_index;
	n = *(short *)cmd_table[keyw_index].ptr;
	for (i=0; i < 8; i++)
		{
		if (instrument == MR200 && i == 5) return ERROR;
		if (n == (1 << i)) return NO_ERROR;
		}
	return ERROR;
}

short t_int (struct commands *cmd_table, short keyw_index, char *line,
			 short parm_index)
{
short tb_int[] = {3, 5, 10, 11};
short i, n;

	line;
	parm_index;
	n = *(short *)cmd_table[keyw_index].ptr;
	for (i=0; i < sizeof(tb_int)/2; i++)
		{
		if (tb_int[i] == n) return NO_ERROR;
		}
	return ERROR;
}

short t_io (struct commands *cmd_table, short keyw_index, char *line,
			 short parm_index)
{
short tb_io[] = {0x300, 0x310, 0x320};
short i, n;

	line;
	parm_index;
	n = *(short *)cmd_table[keyw_index].ptr;
	for (i=0; i < sizeof(tb_io)/2; i++)
		{
		if (tb_io[i] == n) return NO_ERROR;
		}
	return ERROR;
}

struct commands cmd_tab[] =
	{
	{"flip display", 	  NO_DUP,  T_VOID, 		 true,  0, &disp_dir, 0, 0},
										// Experience setup
	{"file prefix",		  NO_DUP,  T_STRING, 	 NULL,  1, &prefix, 5, 0},
	{"# of sequences",	  NO_DUP,  T_LONG_MMAX,	 NULL,  1, &n_seq,  100000l, 1},
	{"# of scans",		  NO_DUP,  T_LONG_MMAX,	 NULL,  1, &n_scans, 65535l, 0},
	{"coad delay",		  NO_DUP,  T_FLOAT_MMAX, NULL,  1, &coad_delay, 1e5, 0},
	{"data type",		  NO_DUP,  T_INT_MMAX,	 NULL,  1, &data_type, 2, 0},
										// Instrument setup
	{"instrument",		  NO_DUP,  T_INT,  		 t_ins, 1, &instrument, 0, 0},
	{"resolution",		  NO_DUP,  T_INT,  		 t_res, 1, &resolution, 0, 0},

	{"channel a present", NO_DUP,  T_VOID, 		 true,  0, &channel_a, 0, 0},
	{"oversampling a", 	  NO_DUP,  T_VOID, 		 true,  0, &over_a, 0, 0},
	{"gain channel a",	  NO_DUP,  T_INT,		 t_res, 1, &gain_a,  0, 0},
	{"detector a delay",  NO_DUP,  T_FLOAT_MMAX, NULL,  1, &delay_a, 25, 0},
	{"sigma minimum a",	  NO_DUP,  T_FLOAT_MMAX, NULL,  1, &sna,     7899., 0},
	{"sigma maximum a",	  NO_DUP,  T_FLOAT_MMAX, NULL,  1, &sxa,     7899., 0},

	{"channel b present", NO_DUP,  T_VOID, 		 true,  0, &channel_b, 0, 0},
	{"oversampling b", 	  NO_DUP,  T_VOID, 		 true,  0, &over_b, 0, 0},
	{"gain channel b",	  NO_DUP,  T_INT,  		 t_res, 1, &gain_b,  0, 0},
	{"detector b delay",  NO_DUP,  T_FLOAT_MMAX, NULL,  1, &delay_b, 25, 0},
	{"sigma minimum b",	  NO_DUP,  T_FLOAT_MMAX, NULL,  1, &snb,     7899., 0},
	{"sigma maximum b",	  NO_DUP,  T_FLOAT_MMAX, NULL,  1, &sxb,     7899., 0},

	{"adc maximum",		  NO_DUP,  T_FLOAT_MMAX, NULL,  1, &adc_max, 5, 0},
	{"laser frequency",   NO_DUP,  T_DOUBLE,	 NULL,  1, &laser, 0, 0},

										// SEQ32 configuration
	{"interrupt request", NO_DUP,  T_INT,		 t_int, 1, &irq, 0, 0},
	{"SEQ32 base",        NO_DUP,  T_INT,		 t_io,  1, &seq32_base, 0, 0}
	};



void error (char *txt);
float irgb (long irgb_time);

long start_irgb;
char path[80];
char txt[80], txt1[80];
char file[80], file1[80], file2[80], file3[80], file4[80];
Lc_handle hdl1, hdl2, hdl3, hdl4;

//----------------------------------------------------------------------------
//	Main program
//----------------------------------------------------------------------------
main (short narg, char *arg[])
{
YDATA y, yy;
short line;
struct date dat;
struct time tim;
short iyear, ims;
short year;
char imonth, iday, ihour, imin, isec;
char month, day, hour, minute;

//----------------------------------------------------------------------------
//	Process configuration file
//----------------------------------------------------------------------------
	// Copy program location into global path variable so we can find files
	str0cpy (path, arg[0], strlen(arg[0])-strlen(strrchr(arg[0],'\\'))+1);

	strcat (path, "fcoad.cfg");
	if (read_setup (path, &line, cmd_tab) != NO_ERROR)
		{
		printf("\nError at line %d in %s file", line, path);
		return 1;
		}
	strrchr(path, '\\')[1]='\0';

	if (narg == 2) str0cpy (prefix, arg[1], 5);
	else if (narg != 1)
		{
		printf ("\nUsage:  fcoad [prefix]");
		return 1;
		}

	y.buffer = (float HPTR *)bo_alloc (max_int_len*4l);
	if (NULL == y.buffer)
		{
		printf ("\nUnable to allocate curve reception buffer");
		return 1;
		}
	y.npts   = max_int_len;				// Communication buffer
	y.firstx = 0;
	y.lastx  = y.npts-1;
//----------------------------------------------------------------------------
//	Get current instrument setup
//----------------------------------------------------------------------------
Mb200_setup setup; 						// MB200 next setup
Mb200_setup dum_set; 					// MB200 use to extract direction info
Mb_status status, stat;					// MB200 curreent status
Op_cond op_cond;						// MB200 operating conditions
long cmd[MB_STATUS_LEN];
long answer_len;

	printf ("\nLoading ucode");
	strcat (path, "sin_cos.bin");
	if (NO_ERROR != load_seq32 (TRUE, seq32_base, path))
		{
		printf ("\nUnable to load ucode file %s", path);
		return 1;
		}
	strrchr(path, '\\')[1]='\0';

	strcat (path, (instrument == MR100) ? "coamb100.bin" : "coamb200.bin");
	if (NO_ERROR != load_seq32 (TRUE, seq32_base, path))
		{
		printf ("\Unable to load ucode file %s", path);
		return 1;
		}
	strrchr(path, '\\')[1]='\0';

	outport (seq32_base, 0);

	printf ("\nSetup IRGB");
	cmd[0] = CFLG_IRGB;
	if (NO_ERROR != seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF))
		{
		printf ("\nUnable to load new requested setup");
		return 1;
		}

	while (!(inport (seq32_base) & HST_M1))
		{
		}
	getdate (&dat);
	iyear  = dat.da_year;
	imonth = dat.da_mon;
	iday   = dat.da_day;

	gettime (&tim);
	ihour   = tim.ti_hour;
	imin = tim.ti_min;

	start_irgb = (3600l * ihour + 60l * imin + tim.ti_sec) * 10000l;

	inportb (seq32_base);				// Reset low/high logic
	outport (seq32_base+2, (short)start_irgb);
	outport (seq32_base+2, (short)(start_irgb >> 16));
	outport (seq32_base, HST_PC1);

	while (inport (seq32_base) & HST_M1)
		{
		}
	outport (seq32_base, 0);

	printf ("\nRead instrument current configuration");
	cmd[0] = CFLG_STAT;
	if (NO_ERROR != seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF))
		{
		printf ("\nUnable to get current setup");
		return 1;
		}

	while (!(inport (seq32_base) & HST_M1))
		{
		}

	if (NO_ERROR !=
		seq32_get_data (&status, MB_STATUS_LEN, &answer_len)  ||
		answer_len != MB_STATUS_LEN)
		{
		printf ("\nUnable to get current setup");
		return 1;
		}
	decode_status (instrument, status.status, &setup);	// Decode MB setup

	if (instrument == MR100)
		{
		if (setup.resolution != resolution)
			{
			printf ("\nIncompatible resolution");
			return 1;
			}
		if (setup.a.present && !channel_a)
			{
			printf ("\nFCOAD.CFG said detector A not present");
			return 1;
			}
		if (!setup.a.present && channel_a)
			{
			printf ("\nFCOAD.CFG said detector A present");
			return 1;
			}
		if (setup.a.present && (setup.a.over != over_a))
			{
			printf ("\nDetector A oversampling state is incorrect");
			return 1;
			}
		if (setup.b.present && !channel_b)
			{
			printf ("\nFCOAD.CFG said detector B not present");
			return 1;
			}
		if (!setup.b.present && channel_b)
			{
			printf ("\nFCOAD.CFG said detector B present");
			return 1;
			}
		if (setup.b.present && (setup.b.over != over_b))
			{
			printf ("\nDetector B oversampling state is incorrect");
			return 1;
			}
		}
	printf ("\nConfigure instrument for experiment");
	setup.resolution = resolution;
	setup.a.present	 = channel_a;
	setup.a.over	 = over_a;
	setup.a.gain	 = gain_a;
	setup.a.delay	 = delay_a;
	setup.b.present	 = channel_b;
	setup.a.over	 = over_a;
	setup.b.gain	 = gain_b;
	setup.b.delay	 = delay_b;
	mb_cmd (instrument, setup, cmd);


	if (NO_ERROR != seq32_tx_data (cmd, 6, LOAD_X, 0))
		{
		printf ("\nUnable to load new requested setup");
		return 1;
		}

	while (!(inport (seq32_base) & HST_M1))
		{
		}
	if (NO_ERROR !=
		seq32_get_data (&status, MB_STATUS_LEN, &answer_len)  ||
		answer_len != MB_STATUS_LEN)
		{
		printf ("\nUnable to load new setup");
		return 1;
		}
	decode_status (instrument, status.status, &setup);	// Decode MB setup
//----------------------------------------------------------------------------
//	Build screen
//----------------------------------------------------------------------------
	if (!vga)							// Allow EGA for old timer
		{
		scr.y2 = 349;
		Win_base0 dummy_window (BO_EGA_COLOR, scr);
		}

	Win_base wmain (scr.x1, scr.y1, scr.x2, scr.y2,
					 border_color, title_back_color,
					 "Fast acquisition", title_color, title_back_color);
	wmain.draw ();
	Dcoord cl = wmain.client_area ();

	short dys = cl.y2 - 5 * CHAR_BHEIGHT;
	Win_curve spc (cl.x1-1, cl.y1, cl.x2+1, dys,
				   border_color, BO_BLACK,
				   NULL, 0, 0,
				   cur_color,
				   scale_color, scale_back_color,
				   6, "cm-1",
				   6, NULL, TRUE);
	Dcoord dspc = spc.curve_area ();
	Wcoord w;
//-----------------------------------------------------------------------------
//	Main display loop
//-----------------------------------------------------------------------------
double tmp;
long s1, s2, npts;
char new_cflags;
char stat_req;
char align;
char start_acq;
char c;

	yy.buffer = (float HPTR *)bo_alloc (max_int_len/2 * 4l);
	if (NULL == yy.buffer)
		{
		printf ("\nUnable to allocate curve reception buffer");
		return 1;
		}
	yy.npts   = max_int_len/2;
	yy.firstx = 0;
	yy.lastx  = yy.npts-1;

deb:
	op_cond.interf_scale = adc_max / 32768.;
	op_cond.n_seq 		 = n_seq;
	op_cond.n_scans 	 = n_scans;
	op_cond.coad_delay   = coad_delay * 10000l;
	op_cond.spec_1st	 = 400;
	op_cond.spec_len	 = 1000;
	op_cond.spec_apod	 = 0;
	op_cond.phase_len	 = 128;
	op_cond.phase_apod   = 0;

	op_cond.cflags = CFLG_OP | CFLG_CSPEC | CFLG_PHASE | CFLG_DIR_0;
	if (setup.a.present) op_cond.cflags |= CFLG_CHANNEL_A;

	if (NO_ERROR !=
		seq32_tx_data (&op_cond, 10, LOAD_X, OP_COND_OFF))
		{
		printf ("\nUnable to load acquisition parameters");
		return 1;
		}
	op_cond.cflags &= ~CFLG_OP;
	while (!(inport (seq32_base) & HST_M1))
		{
		}
	if (NO_ERROR !=
		seq32_get_data (&status, MB_STATUS_LEN, &answer_len)  ||
		answer_len != MB_STATUS_LEN)
		{
		printf ("\nUnable to load new setup");
		return 1;
		}
	decode_status (instrument, status.status, &setup);	// Decode MB setup

	new_cflags = FALSE;
	stat_req   = FALSE;
	align	   = TRUE;
	start_acq  = FALSE;

	do
		{
		do
			{
			if (kbhit ())					// Handle user commands
				{
				c = toupper (getch ());

				switch (c)
					{
					case '\033':			// Exit
					case 'Q':
						exit (0);

					//
					//	cflags bits
					//
					case 'A':               // Channel A
						if (setup.a.present)
							{
							op_cond.cflags |= CFLG_CHANNEL_A;
							new_cflags = TRUE;
							}
						break;

					case 'B':               // Show channel B
						if (setup.b.present)
							{
							op_cond.cflags &= (~CFLG_CHANNEL_A);
							new_cflags = TRUE;
							}
						break;

					case 'H':
						op_cond.cflags |= CFLG_SOFT_ABORT;
						new_cflags = TRUE;
						break;

					case 'P':
						outport (seq32_base, inport(seq32_base) & ~HST_PC2);
						break;

					case 'S':
						start_acq = TRUE;
						break;
				 	}
				}
			if (!stat_req && new_cflags)
				{
				new_cflags = FALSE;

				if (NO_ERROR !=
					seq32_tx_data (&op_cond, sizeof(op_cond)/4, LOAD_X,
								   OP_COND_OFF))
					{
					error ("Unable to update flags");
					}
				}

			if (!stat_req)
				{
				if (start_acq)
					{
					start_acq = FALSE;
					align = FALSE;
					outport (seq32_base, inport(seq32_base) | HST_PC2);
					}
				stat_req = TRUE;
				op_cond.cflags &= ~CFLG_ALIGN;
				op_cond.cflags &= ~CFLG_CPY;
				cmd[0] = op_cond.cflags | (align ? CFLG_ALIGN : CFLG_CPY);
				if (NO_ERROR !=  seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF))
					{
					error ("Unable to request a new status");
					}
				}
			} while (!(inport (seq32_base) & HST_M1));
//-----------------------------------------------------------------------------
//	Process snap shot
//-----------------------------------------------------------------------------

		if (NO_ERROR !=
			seq32_rx_data (&stat, MB_STATUS_LEN, DUMP_X, 0))
			{
			error ("\nGet status error");
			}
		decode_status (instrument, stat.status, &setup);	// Decode MB setup\

		npts = 16384l / setup.resolution;
		tmp  = 2 * npts / laser;
		if (op_cond.cflags & CFLG_CHANNEL_A)
			{
			if (setup.a.over) tmp *= 2;
			s1 = (long)sna * tmp;
			s2 = (long)sxa * tmp + 0.99;
			}
		else
			{
			if (setup.b.over) tmp *= 2;
			s1 = (long)snb * tmp;
			s2 = (long)sxb * tmp + 0.99;
			}
		if (s1 >= npts) s1 = 0;
		if (s2 >= npts) s2 = npts - 1;
		op_cond.spec_1st = s1;
		op_cond.spec_len = s2 - s1 + 1;
		y.firstx = s1 / tmp;
		y.lastx  = s2 / tmp;

		if (NO_ERROR !=
			seq32_tx_data (&op_cond, sizeof(op_cond)/4-1, LOAD_X, OP_COND_OFF))
			{
			error ("\nSet spec limits error");
			}

		stat_req = FALSE;

		if (align)						// If align mode, read ZPD region
			{							// then forget it
			if (NO_ERROR !=
				seq32_get_data (y.buffer, max_int_len, &answer_len)	||
				answer_len != 64)
				{
				error ("\nGet ZPD region error");
				}

			while (!(inport (seq32_base) & HST_M1))
				{
				}
			}

		if (NO_ERROR !=
			seq32_get_data (&status, MB_STATUS_LEN, &answer_len) ||
			answer_len != MB_STATUS_LEN)
			{
			error ("\nGet status error");
			}

		while (!(inport (seq32_base) & HST_M1))
			{
			}

		if (NO_ERROR !=
			seq32_get_data (y.buffer, max_int_len, &y.npts))
			{
			error ("\nGet spectrum region error");
			}

		if (!align && n_scans != 0)
			{
			while (!(inport (seq32_base) & HST_M1))
				{
				}

			if (NO_ERROR !=
				seq32_get_data (yy.buffer, max_int_len, &yy.npts))
				{
				error ("\nGet spectrum region error");
				}
			yy.firstx = y.firstx;
			yy.lastx  = y.lastx;
			v_add (y.buffer, yy.buffer, y.buffer, y.npts);
			}

			sprintf (txt, "%s   resol:%-3d cm-1  seq:%5ld"
					 "  dir0:%5ld  dir1:%5ld  bad:%5ld",
					 (instrument == MR100) ? "MR100" : "MR200",
					 setup.resolution, status.seq_ctr,
					 status.scans_0, status.scans_1, status.scans_bad);
		dchr_col (title_color, title_back_color);
		dcur_pos (cl.x1+CHAR_WIDTH, dys+4);
		db_puts (txt);

		dchr_col (COLOR_A, title_back_color);
		dcur_pos (cl.x1+CHAR_WIDTH, dys + 4 + CHAR_BHEIGHT);
		if (setup.a.present)
			{
			if (instrument == MR100)
				{
				sprintf (txt, "Det A:  %s"
						  "                       ",
						  setup.a.over ? "SW" : "LW");
				}
			else
				{
				gformat (setup.a.delay, 5, txt1);
				sprintf (txt, "Det A:  %s  ID: %-3d  Gain: %-2d  Delay:  %-5sus"
						  "                       ",
						  setup.a.over ? "SW" : "LW",
						  setup.a.id, setup.a.gain, txt1);
				}
			db_puts (txt);
			dchr_col (title_color, BO_RED);
			dcur_pos (cl.x1+45*CHAR_WIDTH, dys + 4 + CHAR_BHEIGHT);
			if		(setup.a.sat_1st)  db_puts ("Signal too high");
			else if (setup.a.sat_last) db_puts ("Gain too high");
			}
		else
			{
			db_puts ("Det A:  Not present                                   "
					 "          ");
			}

		dchr_col (COLOR_B, title_back_color);
		dcur_pos (cl.x1+CHAR_WIDTH, dys + 4 + 2*CHAR_BHEIGHT);
		if (setup.b.present)
			{
			if (instrument == MR100)
				{
				sprintf (txt, "Det B:  %s"
						  "                       ",
						  setup.b.over ? "SW" : "LW");
				}
			else
				{
				gformat (setup.b.delay, 5, txt1);
				sprintf (txt, "Det B:  %s  ID: %-3d  Gain: %-2d  Delay:  %-5sus"
						  "                       ",
						  setup.b.over ? "SW" : "LW",
						  setup.b.id, setup.b.gain, txt1);
				}
			db_puts (txt);

			dchr_col (title_color, BO_RED);
			dcur_pos (cl.x1+45*CHAR_WIDTH, dys + 4 + 2*CHAR_BHEIGHT);
			if 		(setup.b.sat_1st)  db_puts ("Signal too high");
			else if (setup.b.sat_last) db_puts ("Gain too high");
			}
		else
			{
			db_puts ("Det B:  Not present                                   "
					 "               ");
			}

		long tirgb = stat.irgb;
		if (tirgb > (24l * 3600l * 10000l)) tirgb -= (24l * 3600l * 10000l);
		ihour  = (char)(tirgb / (3600l * 10000l));
		tirgb -= (long)ihour * 3600l * 10000l;
		imin   = (char)(tirgb / (60l * 10000l));
		tirgb -= (long)imin * 60l * 10000l;
		isec   = (char)(tirgb / 10000l);
		tirgb -= (long)isec * 10000l;
		ims	   = (short)(tirgb / 10);

		long tdelay;
		if ((stat.flags & FLG_DELAY) && !align && stat.delay)
			{
			tirgb = stat.irgb - stat.irgb_start;
			if (tirgb < 0) tirgb += 24l * 3600l * 10000l;

			tdelay = stat.delay - tirgb;
			}
		else
			{
			tdelay = 0;
			}
		if (tdelay < 0) tdelay = 0;

		sprintf (txt, "IRGB:  %02d:%02d:%02d.%03d  delay:%6ld.%03ld          ",
				 ihour, imin, isec, ims,
				 tdelay/10000l, (tdelay % 10000l) / 10l);

		dchr_col (title_color, title_back_color);
		dcur_pos (cl.x1+CHAR_WIDTH, dys+4+3*CHAR_BHEIGHT);
		db_puts (txt);
		if (setup.bad_scan) continue;

 		if (!(setup.a.present || setup.b.present)) continue;

		cur_color = (status.op_cond.cflags & CFLG_CHANNEL_A) ? COLOR_A :
															   COLOR_B;
		spc.cur_color (cur_color);

		w.x1 = disp_dir ? y.lastx  : y.firstx;
		w.x2 = disp_dir ? y.firstx : y.lastx;
		w.y1 = 1;
		w.y2 = 0;
		spc.world (w);

		spc.update (&y, TRUE, TRUE);

		} while (!(inport (seq32_base) & HST_M2));
		outport (seq32_base, 0);
//-----------------------------------------------------------------------------
//	Save the acquired data
//-----------------------------------------------------------------------------
short direction;

Scan_hdr hdr;
float second;
short det, det1, det2, n_files;
short i;
char  res[9], memo[100];
char  x_type, y_type, z_type, sc_format, not_first;


	det1 = channel_a ? 0 : 1;
	det2 = channel_b ? 1 : 0;

	sc_format = n_seq == 1;
	z_type	  = SECONDS;
	sprintf (res, "%dcm-1", resolution);
	strcpy (memo, "");
	for (det=det1; det <= det2; det++)
		{
		sprintf (file, "%s%03d", prefix, file_ctr);

		if (NO_ERROR != seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0))
			{
			error ("\nUnable to get status");
			}

		npts = 16384l / setup.resolution;
		if (data_type)
			{
			tmp  = 2 * npts / laser;
			if (det == 0)
				{
				if (setup.a.over) tmp *= 2;
				s1 = (long)sna * tmp;
				s2 = (long)sxa * tmp + 0.99;
				}
			else
				{
				if (setup.b.over) tmp *= 2;
				s1 = (long)snb * tmp;
				s2 = (long)sxb * tmp + 0.88;
				}
			if (s1 >= npts) s1 = 0;
			if (s2 >= npts) s2 = npts - 1;
			op_cond.spec_1st = s1;
			op_cond.spec_len = s2 - s1 + 1;
			y.npts = op_cond.spec_len;
			y.firstx = s1 / tmp;
			y.lastx  = s2 / tmp;
			}
		else
			{
			y.npts   = 2l * npts;
			y.firstx = 0;
			y.lastx  = laser / 2;
			}
		//
		//	Send EOA with requested data type
		//
		op_cond.cflags = CFLG_EOA;
		if (det == 0) op_cond.cflags |= CFLG_CHANNEL_A;

		switch (data_type)
			{
			case 0:						// Interferogram
				op_cond.cflags |= 0;
				x_type = ARBITRARY;
				y_type = INTERFEROGRAM;


				sprintf (file1, "%s.i0%1c", file, "AB"[det]);
				n_files = 1;
				if (n_scans)
					{
					sprintf (file2, "%s.i1%1c", file, "AB"[det]);
					n_files = 2;
					}
				break;

			case 1:						// Complex spectrum
				op_cond.cflags |= CFLG_CSPEC;
				x_type = WAVENUMBERS;
				y_type = VOLTS;

				if (n_scans)
					{
					sprintf (file1, "%s.C0%1c", file, "AB"[det]);
					sprintf (file2, "%s.C1%1c", file, "AB"[det]);
					sprintf (file3, "%s.C2%1c", file, "AB"[det]);
					sprintf (file4, "%s.C3%1c", file, "AB"[det]);
					n_files = 4;
					}
				else
					{
					sprintf (file1, "%s.CR%1c", file, "AB"[det]);
					sprintf (file2, "%s.CI%1c", file, "AB"[det]);
					n_files = 2;
					}
				break;

			case 2:						// Raw spectrum
				op_cond.cflags |= CFLG_CSPEC | CFLG_PHASE;
				x_type = WAVENUMBERS;
				y_type = VOLTS;


				sprintf (file1, "%s.RW%1c", file, "AB"[det]);
				n_files = 1;
				break;
			}

		if (NO_ERROR !=
			seq32_tx_data (&op_cond, sizeof(op_cond)/4, LOAD_X, OP_COND_OFF))
			{
			error ("\nSet spec limits error");
			}

		while (!(inport (seq32_base) & HST_M1))
			{
			}
		if (NO_ERROR !=
			seq32_get_data (&hdr, SCAN_HDR_LEN, &answer_len)	||
			answer_len != SCAN_HDR_LEN)
			{
			error ("Error while transfering header");
			}
		time_t file_time =
			bo_get_time_t (iyear, imonth, iday, ihour, imin, 0) +
			(long)(irgb (hdr.irgb_start) / 60);

		time_t start_time = (ihour * 3600l + imin * 60l) * 10000l;
		struct tm *ttm = localtime (&file_time);
		year   = ttm->tm_year;
		month  = ttm->tm_mon;
		day    = ttm->tm_mday;
		hour   = ttm->tm_hour;
		minute = ttm->tm_min;

		switch (n_files)
			{
			case 4:
				if (NO_ERROR !=
					gal_open (TRUE, file4, &hdl4,
							  y.npts, y.firstx, y.lastx,
							  &x_type, &y_type, &z_type,
							  &year, &month, &day, &hour, &minute,
							  res, memo, &sc_format))
					{
					error ("\nError during open 4");
					}
			case 3:
				if (NO_ERROR !=
					gal_open (TRUE, file3, &hdl3,
							  y.npts, y.firstx, y.lastx,
							  &x_type, &y_type, &z_type,
							  &year, &month, &day, &hour, &minute,
							  res, memo, &sc_format))
					{
					error ("\nError during open 3");
					}
			case 2:
				if (NO_ERROR !=
					gal_open (TRUE, file2, &hdl2,
							  y.npts, y.firstx, y.lastx,
							  &x_type, &y_type, &z_type,
							  &year, &month, &day, &hour, &minute,
							  res, memo, &sc_format))
					{
					error ("\nError during open 2");
					}
			case 1:
				if (NO_ERROR !=
					gal_open (TRUE, file1, &hdl1,
							  y.npts, y.firstx, y.lastx,
							  &x_type, &y_type, &z_type,
							  &year, &month, &day, &hour, &minute,
							  res, memo, &sc_format))
					{
					error ("\nError during open 1");
					}
			}

		direction = 0;
		not_first = FALSE;
		for (i=0; i < status.seq_ctr; i++)
			{
			sprintf (txt,"Saving in %s detector %1c sequence #%d   ",
						file, 'A'+det, i);
			dcur_pos (cl.x1+CHAR_WIDTH, dys + 4 + 3*CHAR_BHEIGHT);
			db_puts (txt);

			if (not_first)
				{
				while (!(inport (seq32_base) & HST_M1))
					{
					}
				if (NO_ERROR !=
					seq32_get_data (&hdr, SCAN_HDR_LEN, &answer_len)	||
					answer_len != SCAN_HDR_LEN)
					{
					error ("Error while transfering header");
					}
				}
			not_first = TRUE;

			if (n_scans == 0)
				{							// Decode MB dir
				decode_status (instrument, hdr.status, &dum_set);
				if (direction != dum_set.dir) error ("Invalid direction");
				direction = direction ? 0 : 1;
				}

			second = (hdr.irgb - start_time) / 10000.;

			while (!(inport (seq32_base) & HST_M1))
				{
				}
			if (NO_ERROR !=
				seq32_get_data (y.buffer, y.npts, &answer_len)	||
				answer_len != y.npts)
				{
				error ("Error while transfering data");
				}
			if (data_type != 2)
				{
				if (NO_ERROR !=  gal_write (hdl1, i, y, second))
					{
					error ("\nError during write");
					}
				}
			switch (data_type)
				{
				case 0:
					if (n_scans)
						{
						while (!(inport (seq32_base) & HST_M1))
							{
							}
						if (NO_ERROR !=
							seq32_get_data (y.buffer, y.npts, &answer_len)	||
							answer_len != y.npts)
							{
							error ("Error while transfering data");
							}
						second = irgb (hdr.irgb_start);

						if (NO_ERROR !=  gal_write (hdl2, i, y, second))
							{
							error ("\nError during write");
							}
						}
					break;

				case 1:
					while (!(inport (seq32_base) & HST_M1))
						{
						}
					if (NO_ERROR !=
						seq32_get_data (y.buffer, y.npts, &answer_len)	||
						answer_len != y.npts)
						{
						error ("Error while transfering data");
						}

					if (NO_ERROR !=  gal_write (hdl2, i, y, second))
						{
						error ("\nError during write");
						}
					if (n_scans)
						{
						while (!(inport (seq32_base) & HST_M1))
							{
							}
						if (NO_ERROR !=
							seq32_get_data (y.buffer, y.npts, &answer_len)	||
							answer_len != y.npts)
							{
							error ("Error while transfering data");
							}

						if (NO_ERROR !=  gal_write (hdl3, i, y, second))
							{
							error ("\nError during write");
							}

						while (!(inport (seq32_base) & HST_M1))
							{
							}
						if (NO_ERROR !=
							seq32_get_data (y.buffer, y.npts, &answer_len)	||
							answer_len != y.npts)
							{
							error ("Error while transfering data");
							}

						if (NO_ERROR !=  gal_write (hdl4, i, y, second))
							{
							error ("\nError during write");
							}

						}
					break;

				case 2:
					if (n_scans)
						{
						while (!(inport (seq32_base) & HST_M1))
							{
							}
						yy.npts = y.npts;			// Bug fix
						if (NO_ERROR !=
							seq32_get_data (yy.buffer, yy.npts, &answer_len)	||
							answer_len != y.npts)
							{
							error ("Error while transfering data");
							}
						v_add (y.buffer, yy.buffer, y.buffer, y.npts);
						}

					if (NO_ERROR !=  gal_write (hdl1, i, y, second))
						{
						error ("\nError during write");
						}
					break;

				}
			}
		switch (n_files)
			{
			case 4:
				gal_close (hdl4);
			case 3:
				gal_close (hdl3);
			case 2:
				gal_close (hdl2);
			case 1:
				gal_close (hdl1);
			}

		}
	file_ctr++;
	goto deb;
}
void error (char *txt)
{
short i, x1, x2, y2;

	i  = strlen (txt);
	if (i < 27) i = 27;
	x1 = (640 - (i+2)*CHAR_WIDTH) / 2;
	x2 = x1 + (i+2)*CHAR_WIDTH;
	y2 = 200 + 4*CHAR_BHEIGHT;
	dfill_col (BO_RED);
	dfill (x1, 200, x2, y2);
	dline_col (BO_WHITE);
	dhline (x1, 200, x2);
	dhline (x1, y2, x2);
	dvline (x1, 200, y2);
	dvline (x2, 200, y2);

	dchr_col (BO_WHITE, BO_RED);
	dcur_pos (x1 + CHAR_WIDTH, 200 + CHAR_BHEIGHT);
	db_puts (txt);
	dcur_pos (x1 + CHAR_WIDTH, 200 + 2*CHAR_BHEIGHT);
	db_puts ("Press any key to continue");
	getch ();
	exit (1);
}
float irgb (long irgb_time)
{

	irgb_time -= start_irgb;
	return irgb_time / 10000.;
}


