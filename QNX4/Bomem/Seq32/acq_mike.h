/* ACQ_MIKE.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/* This software is the property of Bomem and should be considered and      */
/* treated as proprietary information.  Refer to the "Source Code License   */
/* Agreement"                                                               */
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 ACQ_MIKE.H 7-Jul-93,10:19:02,`THOMAS' File creation
1:1 ACQ_MIKE.H 15-Apr-94,9:29:22,`JEAN' Added the new TLIB header
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

/* Include for acquisition with a DSP or DMA board */

#ifndef	BOMEM_ACQ_MIKE
#define	BOMEM_ACQ_MIKE

#ifndef	BOMEM_USEFUL
	#include "USEFUL.H"
#endif

#ifndef	CLK_TCK
	#include <time.h>
#endif


/* Instrument type */
#define	M100				0			/* M100 or M102 */
#define	MB					1			/* MB Series + M150 */
#define	M120				2			/* M120 */
#define MB_RAMAN			3			/* MB Raman */
#define M110                4           /* M110, dma only */
#define MB200				5			/* MB 200 fast scan */

/* The following define are to be added to the instrument type */
#define DOUBLE_BUFFER		128			/* Define double buffer mode */
#define	KEEP_FIRST			64			/* Keep data from the first scan */
										/* even if a portion of it may have */
										/* been acquired prior to the */
										/* acquisition start */


/* Type of acquisition card */
#define DMA_BOARD			0			/* Standard DMA board */ 
#define DSP_BOARD			1			/* DSP 100 board */
#define AUTO_DETECT			2			/* Select the installed board */
#define DSP_96000			5			/* DSP 96000 board */
#define NDMA_BOARD			6			/* New serial link DMA board */

/* Various specific error messages */
#define ERROR_SETUP		(ERROR_BASE_ACQ)	/* Invalid setup parameters */
#define ERROR_START		(ERROR_BASE_ACQ-1)	/* Install or setup not done */
#define ERROR_ACQ		(ERROR_BASE_ACQ-2)	/* Interferometer setup has */
											/* changed during acquisition */
#define ERROR_NO_ACQ	(ERROR_BASE_ACQ-3)	/* No acquisition in progress */
#define ERROR_DSP		(ERROR_BASE_ACQ-4)	/* Communication problem with DSP */
#define ERROR_OVERRUN	(ERROR_BASE_ACQ-5)	/* DSP overrun */
#define ERROR_DSPCT		(ERROR_BASE_ACQ-6)	/* Counter mismatch, bad npts */
											/* May happen if incorrect */
											/* instrument is selected or if */
											/* resolution has changed */
#define ERROR_ACQOF		(ERROR_BASE_ACQ-7)	/* Interrupt rate too fast */
#define ERROR_RANGE		(ERROR_BASE_ACQ-8)	/* Invalid range */
#define ERROR_ILL_SETUP	(ERROR_BASE_ACQ-9)	/* Invalid interferometer config, */
											/* oversampling at 1cm-1 */
#define ERROR_INV_BOARD (ERROR_BASE_ACQ-10) /* Unsupported board */

/* Miscellaneous */
#define PHASE_PTS		256L			/* For 1 sample per fringe */
#define TIMEOUT_DELAY	50L				/* In seconds */

/* ACQ_MIKM.C */
short acq_install (short board_type, short instrument, short interrupt_no,
				   short dma_channel, short io_addr, char *path);
void acq_remove (void);
short acq_wait_end_coad (YDATA *data_buffer, double *acq_time,
						 short (*userwait)(word scans_0, word scans_1,
										   word bad_scans, long acq_num));
short acq_get_coad (YDATA *spectrum, double *acq_time, long *acq_num);
short acq_wait_end_coad0(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r,
						 YDATA *phs_i, double *acq_time,
						 short (*userwait)(word scans_0, word scans_1,
										   word bad_scans, long acq_num));
short acq_get_coad0(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r, YDATA *phs_i,
					double *acq_time, long *acq_num);
short acq_hard_abort (void);
short acq_stat (word *scans_0, word *scans_1, word *scans_bad, long *acq_num);
short acq_status (word *scans_0, word *scans_1, word *scans_bad, long *acq_num,
					short *resolution, short *speed, short det1[4],
					short det2[4], short *auxdet, short *acq_err);
short acq_speed(float *speed);
void set_raman_freq(double frequency);
void set_laser_freq(double frequency);

/* ACQ_MIKI.C */
short acq_get_int (YDATA *interf, word nbr_scans, long nbr_acq, double delay,
                   char wait, double *acq_time,
				   short (*userwait)(word scans_0, word scans_1,
								     word bad_scans, long acq_num));
/* ACQ_MIKS.C */
short acq_get_spec (YDATA *spectrum, word nbr_scans, long nbr_acq,
					double delay, float user_sna, float user_sxa,
					float user_snb, float user_sxb, short apodization,
					char wait, double *acq_time,
				    short (*userwait)(word scans_0, word scans_1,
									  word bad_scans, long acq_num));
short acq_get_raw_spec(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r, YDATA *phs_i,
					word nbr_scans, long nbr_acq, double delay, float user_sna,
					float user_sxa, float user_snb, float user_sxb,
					short apodization, char wait, double *acq_time,
				    short (*userwait)(word scans_0, word scans_1,
									  word bad_scans, long acq_num));
/* ACQ_MIKC.C */
short acq_copy (YDATA *coadbuf, word *scans_0, word *scans_1, long *acq_num);
short acq_copy0(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r, YDATA *phs_i,
					word *scans_0, word *scans_1, long *acq_num);

/* ACQ_TIME.C */
double bo_acq_time(void);

/* ACQ_MIK2.C */
short acq_get_igm_cont (word nbr_scans, long n_acq, double delay,
						short (*userwait)(word scans_0, word scans_1,
						word bad_scans, float time_left),
						short (*userwork)(YDATA igm, time_t acq_time,
						long index, word sc0, word sc1, word bad_sc,
						double acq_dtime));
short acq_get_spec_cont(word nbr_scans, long n_acq, double delay,
						float user_sn, float user_sx, short apodization,
						short (*userwait)(word scans_0, word scans_1,
						word bad_scans, float time_left),
						short (*userspec)(YDATA spectrum, time_t acq_t,
						long index,	word sc0, word sc1, word bad_sc,
						double acq_dtime));
short acq_get_raw_spec_cont(word nbr_scans, long n_acq, double delay,
						float user_sn, float user_sx, short apodization,
						short (*userwait)(word scans_0, word scans_1,
						word bad_scans, float time_left),
						short (*userspec)(YDATA spc_r, YDATA spc_i,
						YDATA phs_r, YDATA phs_i, double acq_t,
						long index,	word sc0, word sc1, word bad_sc));

#endif
