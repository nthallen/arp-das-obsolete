/* SEQ32_PC.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/* This software is the property of Bomem and should be considered and      */
/* treated as proprietary information.  Refer to the "Source Code License   */
/* Agreement"                                                               */
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 DSP96_PC.H 23-Oct-92,16:22:10,`THOMAS' File creation
1:1 DSP96_PC.H 15-Apr-94,9:29:24,`JEAN' Added the new TLIB header
1:2 SEQ32_PC.H 2-May-94,16:27:02,`THOMAS'
     Changed the name from dsp96_pc.h to seq32_pc.h when the hardware design of
     the prototype dsp96000 was replaced with the final design. The prototype
     board is no longer supported in the driver.

     The new file contains all new defines for the new DSP96000 card and uses
     C++ constructs that are not C compatible. It also contains defines for the
     functions that interface with acq_mik? and that have not been changed from
     a prototype point of view.
1:3 SEQ32_PC.H 8-Jun-94,16:57:00,`THOMAS'
     Added support for MB100 with 2 detectors and the DSP96000 board. To do this
     resolutions up to 128cm-1 need to be accepted and commands to change
     the resolution or other parameters need to be refused. Also the speed of the
     instrument is obtained by measuring the length of a scan rather than from
     a table lookup like on the MB200. The way the status of the instrument
     is interpreted also changes and so we need to have two different routines
     to read the status, also the microcode changes depending on the instrument.


1:4 SEQ32_PC.H 2-Aug-94,17:55:46,`THOMAS'
     Fixed a problem with the oversampling indicator in the status on the MB100,
     DSP96000 interface, added 2 new variables in the status block in order
     to transmit the scan start and scan end time reliably. Added some debugging
     tools for tracking down the garbage bug in rx_data, these debugging aids are
     in comments for now and the garbage bug is bypassed by using get_data.
1:5 SEQ32_PC.H 27-Sep-94,13:52:10,`THOMAS'
     Change prototype for dsp96_set_status which is used to set resolution by
     remote control with the DSP96000; added the userwait parameter to allow for
     slow response from the instrument.
1:6 SEQ32_PC.H 28-Sep-94,12:42:34,`JEAN'
     Changed prototype for userwait() parameter in dsp96_set_status()
     prototype.
1:7 SEQ32_PC.H 28-Sep-94,15:07:02,`JEAN'
     Removed a parameter in prototype for userwait() in dsp96_set_status()
     parameter list.
1:8 SEQ32_PC.H 17-Nov-94,12:22:36,`CLAUDE'
     Add rd_ucode.cpp function prototype
     Move ldb_32 function outside assembler function prototypes list
1:9 SEQ32_PC.H 4-Jan-95,14:02:24,`FRAGAL'
     New function to select the channel when using the copy() function.
1:10 SEQ32_PC.H 24-Jan-95,11:35:02,`THOMAS' Add a define for channel_B
1:11 SEQ32_PC.H 13-Feb-95,12:01:04,`CLAUDE' Add seq32_data_ready function
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif



#ifndef	HPTR
	#include "useful.h"
#endif

const long SRAM_AB_OFFSET	= 0x10000000l;// Port A and B static ram base offset
const long SRAM_AB_SIZE		= 256l*1024l;
const long SRAM_A_OFFSET	= 0x20000000l;// Port A static ram base offset
const long SRAM_A_SIZE		= 256l*1024l;
const long SRAM_B_OFFSET	= 0x40000000l;// Port B static ram base offset
const long SRAM_B_SIZE		= 256l*1024l;
const long DRAM_OFFSET		= 0x60000000l;// Dynamic ram base offset
const long DRAM_SIZE		= 16l*1024l*1024l;

const short SEQ32_BASE = 0x300;			// DSP base address
const short HST_LEN    = 1024;			// HST_FIFO length

const short HST_FLG	   = 0; 		 	// RW16 Host flags register
const short HST_RST_LH = 0;  			// R8   Reset low/high logic
const short HST_INT	   = 0;			  	// W8   Signal interrupt to SEQ32
const short HST_REG	   = 2;				// RW16 Host register
const short HST_RESET  = 2;				// R8   Reset SEQ32 board
const short HST_FIFO   = 4;				// R16  Host fifo
//					
//	Flag register bit definitions
//
const short HST_PC0	  =	1;				// RW Written to DSP
const short HST_PC1	  =	2;				// RW Written to DSP
const short HST_PC2	  =	4;				// RW Written to DSP
const short HST_PC3	  =	8;				// RW Written to DSP
const short HST_PC4	  =	0x10;			// RW Written to DSP
const short HST_M0	  =	0x20;			// R  Written by DSP
const short HST_M1	  =	0x40;			// R  Written by DSP
const short HST_M2	  =	0x80;			// R  Written by DSP
const short HST_M3	  =	0x100;			// R  Written by DSP
const short HST_M4	  =	0x200;			// R  Written by DSP
const short HST_M5	  =	0x400;			// R  Written by DSP
const short HST_TXF	  =	0x800;			// R  Host transmit register empty flag
const short HST_RXF	  =	0x1000;			// R  Host receive register full flag
const short HST_FULL  =	0x2000;			// R  /Host fifo full flag
const short HST_HALF  =	0x4000;			// R  /Host fifo half full flag
const short HST_EMPTY =	0x8000;			// R  /Host fifo empty flag


const short FLG_DELAY = 0x10;			// Acquisition delay in progress
const short ST_DIR	  = 0x400;			// 10	Direction bit

//
//	ram_type definitions
//
const short DUMP_X = 0;
const short DUMP_Y = 1;
const short DUMP_P = 2;
const short LOAD_X = 3;
const short LOAD_Y = 4;
const short LOAD_P = 5;

const short OP_COND_OFF		= 6;		// cflags offset
const short CFLG_OFF		= 15;		// cflags offset

const short CFLG_IRGB		= 1;		// New IRGB time
const short CFLG_OP			= 4;		// New operating conditions

const short CFLG_STAT		= 8;		// Current status request
const short CFLG_ALIGN		= 0x10;		// Copy of current scan requested
const short CFLG_CPY		= 0x20;		// Copy of the current coad buffer req
const short CFLG_SOFT_ABORT	= 0x40;		// Soft abort requested
const short CFLG_EOA		= 0x80;		// Request the acquired data

const short CFLG_CHANNEL_B	= 0x000;	// Channel B requested if 0
const short CFLG_CHANNEL_A	= 0x100;	// Channel A requested if 1
const short CFLG_DIR_0		= 0x200;	// Direction 0 requested
const short CFLG_DIR_1		= 0x400;	// Direction 1 requested

const short CFLG_CSPEC		= 0x800;	// Complex spectrum requested
const short CFLG_PHASE		= 0x1000; 	// Phase correction requested

typedef struct
	{
	char  present;						// != 0 if present
	char  over;							// != 0 if oversampling is used
	char  id;							// Detector identification code
	float delay;						// Detector delay in usec
	char  gain;							// Preampl last stage gain [1,2,4,8,16]
	char  gain_1st;						// Preampl first stage, must be 0
	char  sat_1st;						// !=0 Saturation on preampl first stage
	char  sat_last;						// !=0 Saturation on preampl last stage
	} Detector_def;

typedef struct
	{
	unsigned char resolution;			// Resolution [1, 2, 4, 8, 16..128]cm-1
										// unsigned to support 128cm-1
	char  speed;						// Must be 0
	char  dir;							// Direction
	Detector_def a;						// Detector A
	Detector_def b;						// Detector B

	char  bad_scan;						// != 0 if bad scan
	char  err_code;						// MB200 error code
	} Mb200_setup;

typedef struct
	{
	float interf_scale;					// Scale factor use to convert the
										// interferogram in volts
	long n_seq;							// Number of sequence to be done
	long n_scans;						// Number of scans per sequence
	long coad_delay;					// Time delay between coads start
	long spec_1st;						// First useful point in the spectrum
										// base on a 1cm-1 resolution
	long spec_len;						// Number of useful points in the spec
	long spec_apod;						// Spectrum apodization fnc #
	long phase_len;
	long phase_apod;					// Phase spectrum apodization fnc #
	long cflags;						// Communication flags
	} Op_cond;

typedef struct
	{	
	long irgb_start;					// IRGB time of the last EOS before
										// first scan
	long irgb;							// Current scan IRGB time
										// In header, time of the last EOS of the
										// sequence
	long status[3];						// Current scan MB200 status
	long seq_ctr;						// Current sequence number
	long scans_0;						// Direction 0 scan done counter
	long scans_1;						// Direction 1 scan done counter
	long scans_bad;						// Bad scan counter
	} Scan_hdr;
const short SCAN_HDR_LEN = sizeof(Scan_hdr) / sizeof(long);

typedef struct
	{	
										// MB setup commands
	long stat_req[3]; 					// Status requested after mb_setup
	long mb_cmd[3];						// Packed commands (12 bytes)
	Op_cond op_cond; 					// Operating conditions
										// Status packet
	long delay;							// Delay before next sequence start
	long flags;							// System flags
	long tmp_stat[3];					// Current status before check
	long irgb_start;					// IRGB time of the last EOS before
										// first scan
	long irgb;							// Current scan IRGB time
										// In header, time of the last EOS of the
										// sequence
	long status[3];						// Current scan MB200 status
	long seq_ctr;						// Current sequence number
	long scans_0;						// Direction 0 scan done counter
	long scans_1;						// Direction 1 scan done counter
	long scans_bad;						// Bad scan counter
	float zpd_value;					// Current scan ZPD abs value
	long zpd_pos;						// Current scan ZPD position
	long data_min;						// used to determine scan speed
	long data_max;
	} Mb_status;
//patched so that data_min, data_max are not normally part of the status!!
const short MB_STATUS_LEN = sizeof(Mb_status) / sizeof(long) - 2;

#ifdef __cplusplus
extern "C" {
#endif

void seq32_set_base (short seq32_base);
void seq32_reset    (short seq32_base, short flags);

short seq32_rx_data (void HPTR *buffer, long length, short ram_type,
					 long address);

short seq32_tx_data (void HPTR *buffer, long length, short ram_type,
					 long address);

short seq32_bootstrap (void HPTR *buffer, long length);
short seq32_data_ready (void);
short seq32_get_data  (void HPTR *buffer, long buf_len, long *answer_len);


#ifdef __cplusplus
			}
#endif

// prototypes for interface to Bomem lib
short rd_ucode (char *ucode_file, long HPTR **ucode, long *ucode_length,
				short n_digits);

short load_seq32 (char reset, short seq32_base, char *filename);

void dsp96_det_delay(float delaya, float delayb);
short dsp96_install(short instrument, short io_addr, char *path);
void dsp96_remove(void);
short dsp96_wait_end_coad(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r,
						YDATA *phs_i, double *acq_time,
						short (*userwait)(word scans_0, word scans_1,
										  word bad_scans, long acq_num));
short dsp96_get_coad(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r, YDATA *phs_i,
					double *acq_time, long *acq_num);
short dsp96_hard_abort(void);
short dsp96_stat(word *scans_0, word *scans_1, word *scans_bad, long *acq_num);
short dsp96_status(word *scans_0, word *scans_1, word *scans_bad,
					long *acq_num, short *resolution, short *speed,
					short det1[4], short det2[4], short *acq_err);
short dsp96_set_status(short resolution, short speed, short det1[4],
					   short det2[4],
					   short (*userwait) ( word scans_0, word scans_1,
										   word bad_scans ));
short dsp96_get_int(YDATA *interf, word nbr_scans, long nbr_acq, double delay,
						char wait, double *acq_time,
						short (*userwait)(word scans_0, word scans_1,
											word scans_bad, long acq_num));
short dsp96_get_raw_int(YDATA *interf, word nbr_scans, long nbr_acq,
						double delay, char wait, double *acq_time,
						short (*userwait)(word scans_0, word scans_1,
											word scans_bad, long acq_num));
short dsp96_get_spec(YDATA *spec, word nbr_scans, long nbr_acq, double delay,
						float user_sna, float user_sxa, float user_snb,
						float user_sxb, short apodization, char wait,
						double *acq_time,
						short (*userwait)(word scans_0, word scans_1,
											word bad_scans, long acq_num));
short dsp96_get_raw_spec(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r,
						YDATA *phs_i, word nbr_scans, long nbr_acq,
						double delay, float user_sna, float user_sxa,
						float user_snb, float user_sxb, short apodization,
						char wait, double *acq_time,
						short (*userwait)(word scans_0, word scans_1,
											word bad_scans, long acq_num));
short dsp96_copy(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r, YDATA *phs_i,
								word *scans_0, word *scans_1, long *acq_num);
void dsp96_speed(float *speed);
short dsp96_select_detector(short detector);
