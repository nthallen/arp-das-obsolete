//	SEQ32_PC.H

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

const short HST_FLG	   = SEQ32_BASE;  	// RW16 Host flags register
const short HST_RST_LH = SEQ32_BASE;  	// R8   Reset low/high logic
const short HST_INT	   = SEQ32_BASE;  	// W8   Signal interrupt to SEQ32
const short HST_REG	   = SEQ32_BASE+2;	// RW16 Host register
const short HST_RESET  = SEQ32_BASE+2;	// R8   Reset SEQ32 board
const short HST_FIFO   = SEQ32_BASE+4;	// R16  Host fifo
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
	char  resolution;					// Resolution [1, 2, 4, 8, 16]cm-1
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
	long zpd_value;						// Current scan ZPD abs value
	long zpd_pos;						// Current scan ZPD position
	} Mb_status;
const short MB_STATUS_LEN = sizeof(Mb_status) / sizeof(long);

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
short seq32_get_data  (void HPTR *buffer, long buf_len, long *answer_len);

void mb_cmd (const Mb200_setup &s, long mbcmd[6]);
// void (* decode_status) (const long *mb_status, Mb200_setup *s);
short load_seq32 (char reset, short seq32_base, char *filename);

#ifdef __cplusplus
			}	
#endif

#ifdef __WATCOMC__
	#pragma aux (ASM_RTN) ASM_UPR "_^";
	#pragma aux (ASM_UPR) seq32_set_base;
	#pragma aux (ASM_UPR) seq32_reset;
	#pragma aux (ASM_UPR) seq32_rx_data;
	#pragma aux (ASM_UPR) seq32_tx_data;
	#pragma aux (ASM_UPR) seq32_bootstrap;
	#pragma aux (ASM_UPR) seq32_get_data;
#endif

// prototypes for interface to Bomem lib
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
					   short det2[4]);
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

