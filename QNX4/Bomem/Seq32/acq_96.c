/* ACQ_96.C */
/*#$%!i ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/*#$%!i ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 ACQ_96.C 16-Jun-93,17:41:42,`THOMAS' File creation
1:1 ACQ_96.C 2-Dec-93,19:28:36,`THOMAS' Fixed bug in raw spectrum
     acquisition with 96000, old version did not coadd forward and
1:2 ACQ_96.C 29-Mar-94,11:25:56,`JEAN'
     Added the standard file header and TLIB marker.
1:3 ACQ_96.C 14-Apr-94,12:49:12,`JEAN' Change TLIB marker
1:4 ACQ_96.c 14-Apr-94,15:33:20,`JEAN' Added missing # in front of endif
1:5 ACQ_96.C 2-May-94,18:20:24,`THOMAS'
     Made significant changes to accomodate the new DSP board, the prototype
     board will no longer be support be any Bomem drivers and will be phased out.

     The new driver is based on a single microcode base so no longer requires
     switching between different microcode file in order to implement different
     functions, also multi-sequence coadding is correctly supported as well
     as copying the buffer while acquisition is running.
1:6 ACQ_96.C 16-May-94,15:05:20,`THOMAS'
     This code now works with the new micro-code that supports one or two
     detectors, the original release failed unless there were 2 detectors.

     Changed the copy routine to follow the same conventions as end_of_coad and
     added the computation of a time variable bo_acquire_96000_time in all
     status requests in order for the remaining time functions to work properly
     in research acquire!
1:7 ACQ_96.C 9-Jun-94,10:01:52,`THOMAS'
     Added support for MB100 with 2 detectors and the DSP96000 board. To do this
     resolutions up to 128cm-1 need to be accepted and commands to change
     the resolution or other parameters need to be refused. Also the speed of the
     instrument is obtained by measuring the length of a scan rather than from
     a table lookup like on the MB200. The way the status of the instrument
     is interpreted also changes and so we need to have two different routines
     to read the status, also the microcode changes depending on the instrument.


1:8 ACQ_96.C 11-Jul-94,14:54:26,`THOMAS'
     Added correct key sequence in the comment teplates so that the documentation
     for the driver can be extracted automatically with PT.exe when the liobrary
     documentation is generated.
1:9 ACQ_96.C 17-Jul-94,20:34:32,`THOMAS'
     Fixed a problem with the extract key of one of the comment headers, it
     started with
     /#
     instead of
     /"star"#
1:10 ACQ_96.C 3-Aug-94,16:29:10,`THOMAS'
     Fixed a problem with the oversampling indicator in the status on the MB100,
     DSP96000 interface, added 2 new variables in the status block in order
     to transmit the scan start and scan end time reliably. Added some debugging
     tools for tracking down the garbage bug in rx_data, these debugging aids are
     in comments for now and the garbage bug is bypassed by using get_data.
1:11 ACQ_96.C 15-Aug-94,9:12:38,`THOMAS'
     Updated the routines that interpret the status information, also fixed
     some details in handling on info when it is transfered to the PC.
1:12 ACQ_96.C 22-Sep-94,13:39:52,`JEAN'
     Added the include for max() ( BC 3.1 )
1:13 ACQ_96.C 22-Sep-94,15:08:34,`JEAN'
     The value for __BCPLUSPLUS__ was not thew one in the
     doc. Used __TCPLUSPLUS__ instead.
1:14 ACQ_96.C 27-Sep-94,14:16:58,`THOMAS'
     Added the userwait parameter in dsp96_set_status(), it is not yet implemented
     but the idea is that if the Michelson is slow to respond to the status
     change the function should wait for confirmation and poll the userwait
     function so that the computer does not lock. Also the set_status function
     now has a device independant interface through acq_set_status() because
     the new DMA interface (SEQ36) also supports remote control.
1:15 ACQ_96.C 28-Sep-94,14:16:22,`JEAN'
     Change prototype for userwait() parameter in dsp96_set_status()
     Use of default det? settings when det1 and/or det2 are NULL.
1:16 ACQ_96.C 28-Sep-94,14:55:06,`JEAN'
     Removed a parameter in prototype for userwait() in dsp96_set_status()
     parameter list.
1:17 ACQ_96.C 6-Oct-94,10:35:32,`THOMAS'
     Fix error in definition of userwait routine in function dsp96_set_status!!!
1:18 ACQ_96.C 27-Oct-94,8:44:10,`JEAN'
     Remove stupid patch ( explicit path ) to include statement
1:19 ACQ_96.C 9-Dec-94,16:46:10,`CLAUDE'
     Fix single channel acquisition with serial link, only works with channel A
     or both channels.  Change the CHANNEL_A const for ~CHANNEL_A when channel A
     not present.
1:20 ACQ_96.C 4-Jan-95,14:50:08,`FRAGAL'
     New function dsp96_select_detector() to select the channel when using
     the dsp96_copy() function.
1:21 ACQ_96.C 25-Jan-95,16:45:50,`THOMAS'
     All the handshake code that interfaces to the DSP now has timeouts so that
     when an error occurs the driver does not lock. Some debugging code that
     is no longer needed was eliminated in the status transmission code now
     that the ground bounce problem in the DSP is fixed. Also reformated
     some code to be more readable.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>

#if defined(__TCPLUSPLUS__) && (__TCPLUSPLUS__==0x0310)
#include <stdtempl.h>
#endif

#include "useful.h"
#include "vector.h"
#include "bo_timer.h"
#include "filegen.h"
#include "acq_mike.h"
#include "spectrum.h" /* for apodization defs */
#include "seq32_pc.h"
#include "display.h"

/* this global variable is a patch to transfer the time remaining to
   stat0 */
double bo_acquire_96000_time = 0.0;

/* assume 3.5 usec detector delay as a default */
static float det_delays[2] = {3.5, 3.5};

/* DSP communication structures */
static long cmd[MB_STATUS_LEN];
static Mb_status status;
static Mb200_setup setup;
static Scan_hdr header;

/* internal working parameters */
static short io_addr;
static char *uc_path = NULL;
static long copy_channel = CFLG_CHANNEL_A;	/* Select detector A for copy */

static byte num_det;				  /* number of detectors present */
static long cur_acq_num;			  /* current sequence number */

static double firstx[2], p_firstx[2]; /* parameters for current acquisition */
static double lastx[2], p_lastx[2];
static long spec2_1st, spec2_len;     /* parameters for second detector */
static long npts[2], p_npts[2];		  /* # of points in spectrum, phase */
static long size[2], p_size[2];       /* size of spectra, phase */
static word scans;
static short acq_type;				  /* type of acquisition */

static short load_seq32 (char reset, short seq32_base, char *filename)
{
long ucode_len;
long HPTR *ucode;
FILE *infile;
short i;

	/* Open ucode file length and determine its length */
	if ((infile=fopen (filename, "rb")) == NULL) return (ERROR);

	fseek (infile, 0, SEEK_END);
	ucode_len = ftell (infile);
	fseek (infile, 0, SEEK_SET);	/* Point to the beginning of the file */

	if ((ucode = (long HPTR *)bo_alloc(ucode_len)) == NULL)
		{
		fclose (infile);			/* Unable to allocate input buffer */
		return (ERROR);
		}
	if (flread (ucode, ucode_len, infile) != NO_ERROR)
		{
		bo_free(ucode);
		fclose(infile);
		return (ERROR);
		}
	fclose (infile);

	if (reset)
		{
		seq32_reset (seq32_base, 0);
		bo_wait (0.3);
		}

	for (i=0; i < 5; i++)
		{
		if (NO_ERROR == seq32_bootstrap (ucode, ucode_len/4l))
			{
			break;
			}
		}
	bo_free (ucode);

	return (i == 5) ? ERROR  :  NO_ERROR;
}

static void encode_mb100 (const Mb200_setup &s, long mb_cmd[6])
{
short *status = (short *)mb_cmd;
char  *cmd    = (char *)&mb_cmd[3];

	memset (mb_cmd, 0, sizeof(mb_cmd)*6);	// Reset mb_cmd

	status[0]  = 0xde60u;
	status[0] |= 7 - (short)(log(s.resolution) / log(2.) + 0.1); // Resolution
	status[0] |= ((s.speed & 1) << 8);		// Speed setup

	if (!s.a.over)	   				// Oversampling setup
		{
		status[0] |= 0x10;
		}

	status[1]  = 0xde60;
	status[1] |= 7 - (short)(log((double)s.resolution) / log(2.) + 0.1); // Resolution
	status[1] |= ((s.speed & 1) << 8);	// Speed setup
	if (!s.b.over)	   					// Oversampling setup
		{
		status[1] |= 0x10;
		}

	cmd[11] = 0x99;		 			// Key
}


static void decode_mb100 (const long *mb_status, Mb200_setup *s)
{
short *status = (short *)mb_status;

//
//	Decode first status word
//
	s->resolution =  1 << (7 - (status[0] & 0x7));
	s->speed 	  = (status[0] >> 8) & 1;
	s->a.present  =  status[2] & 1;
	s->b.present  =  (status[2] >> 1) & 1;
	s->dir		  =  (status[0] & 0x80) >> 7;
	s->a.over	  =  (status[0] & 0x10) == 0;
	s->b.over	  =  (status[1] & 0x10) == 0;

	s->a.gain	  =  1;
	s->a.gain_1st =  0;
	s->a.sat_1st  =  FALSE;				// Not supported
	s->a.sat_last =  FALSE;				// Not supported
	s->a.id		  =  0;
//
//	Decode second status word
//
	s->b.gain	    =  1;
	s->b.gain_1st	=  0;
	s->b.sat_1st	=  FALSE;			// Not supported
	s->b.sat_last	=  FALSE;			// Not supported
	s->b.id			=  0;

	s->bad_scan	=  (status[0] & 0x40) == 0;
	s->err_code	=  s->bad_scan;
//
//	Decode third status word
//
	s->a.delay = 0;
	s->b.delay = 0;
}

void encode_mb200 (const Mb200_setup &s, long mb_cmd[6])
{
static char tb_res[17]  = {0, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4};
static char tb_gain[17] = {0, 3, 4, 0, 5, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 7};

short *status = (short *)mb_cmd;
char  *cmd    = (char *)&mb_cmd[3];

	memset (mb_cmd, 0, sizeof(mb_cmd)*6);	// Reset mb_cmd
	cmd[0]  = 0xaa;						// Key
	cmd[1]  = 0x55;

	status[0]  = tb_res[s.resolution];	// Resolution setup
	cmd[2]     = tb_res[s.resolution];
	status[0] |= (s.speed << 4);		// Speed setup
	cmd[3]	   =  s.speed;

	if (s.a.present)
		{
		status[0] |= 0x100;
		if (s.a.over)	   				// Oversampling setup
			{
			status[0] |= 0x5000;
			cmd[3] |= 0x10;
			}
		status[1]  =  tb_gain[s.a.gain];// Channel A gain setup
		status[1] |= (s.a.gain_1st << 4);
		cmd[4] 	   =  status[1];

		status[1] |= (s.a.id << 8);		// Detector A identification code setup
		}
	if (s.b.present)
		{
		status[0] |= 0x200;
		if (s.b.over)	   				// Oversampling setup
			{
			status[0] |= 0xa000;
			cmd[3] |= 0x20;
			}
		status[2]  =  tb_gain[s.b.gain];	// Channel A gain setup
		status[2] |= (s.b.gain_1st << 4);
		cmd[5] 	   =  status[2];

		status[2] |= (s.b.id << 8);			// Detector A identification code setup
		}

	if (s.a.delay <= 0.13)				// Channel A delay setup
		{
		status[4] = 0xffff;
		}
	else
		{
		short t = (s.a.delay - 0.13) / 0.05 + 0.5;
		short t1 = t / 2;
		t -= t1;
		status[4] = ((255-t) << 8) | (255-t1);
		}
	cmd[6] =  status[4] & 0xff;
	cmd[7] = (status[4] >> 8) & 0xff;

	if (s.b.delay <= 0.13)				// Channel B delay setup
		{
		status[5] = 0xffff;
		}
	else
		{
		short t = (s.b.delay - 0.13) / 0.05 + 0.5;
		short t1 = t / 2;
		t -= t1;
		status[5] = ((255-t) << 8) | (255 - t1);
		}
	cmd[8] =  status[5] & 0xff;
	cmd[9] = (status[5] >> 8) & 0xff;

	cmd[11] = 0x99;						// Key
}


void decode_mb200 (const long *mb_status, Mb200_setup *s)
{
static char tb_gain[8] = {1, 1, 1, 1, 2, 4, 8, 16};
short *status = (short *)mb_status;
//
//	Decode first status word
//
	s->resolution =  1 << (status[0] & 0x7);
	s->speed 	  = (status[0] >> 4) & 0xf;
	s->a.present  =  (status[0] & 0x100) != 0;
	s->b.present  =  (status[0] & 0x200) != 0;
	s->dir		  =  status[0] & 0x400;
	s->a.over	  =  (status[0] & 0x5000) != 0;
	s->b.over	  =  (status[0] & 0xa000) != 0;

	s->a.gain	  =  tb_gain[status[1] & 0x7];
	s->a.gain_1st =  0;
	s->a.sat_1st  = (status[1] & 0x40) != 0;
	s->a.sat_last = (status[1] & 0x80) != 0;
	s->a.id		  = (status[1] >> 8) & 0xff;
//
//	Decode second status word
//
	s->b.gain	    =  tb_gain[status[2] & 0x7];
	s->b.gain_1st	=  0;
	s->b.sat_1st	= (status[2] & 0x40) != 0;
	s->b.sat_last	= (status[2] & 0x80) != 0;
	s->b.id			= (status[2] >> 8) & 0xff;

	s->bad_scan	=  status[3] & 1;
	s->err_code	= (status[3] >> 1) & 0x7f;
//
//	Decode third status word
//
	s->a.delay = ((255 - (status[4] >> 8) & 0xff) + (255 - status[4] & 0xff)) *
																0.05 + 0.13;
	s->b.delay = ((255 - (status[5] >> 8) & 0xff) + (255 - status[5] & 0xff)) *
																0.05 + 0.13;
}

static void (*encode_status) (const Mb200_setup &s, long mb_cmd[6]);
static void (*decode_status) (const long *mb_status, Mb200_setup *s);
static char i_mb200;

/*#$%!i**************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_DET_DELAY
File:   ACQ_96.C
Author: Thomas Buijs
Date:   October 1992

Synopsis
        #include "dsp96_pc.h"

        void dsp96_det_delay(float delaya, float delayb)
 
        delaya          preamplifier delay in microseconds for detector a
        delayb          preamplifier delay in microseconds for detector b

Description
        This routines allows the preamplifier delays to be set on the MB200.
        This routine must be called before dsp96_install to have an effect,
        if it is not called the default values (3.5 and 3.5) are used
 #$%!i........................................................................*/

void dsp96_det_delay(float delaya, float delayb)
{
	det_delays[0] = delaya;
	det_delays[1] = delayb;
}

/*#$%!i**************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_INSTALL
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        short dsp96_install (short instrument, short _io_addr, char *path)
 
        instrument      1 = MB series
                        3 = MB Raman
                        5 = MB200
                        If the second most significant bit is set, then
                        the first scan collected may correspond to some data
                        acquired just before the acquisition start.  It should
                        not be set when the data synchronization is critical.
                        It should be set whenever continuous data acquisition
                        is required.  
        _io_addr        DSP96000 base i/o address
                        Factory setting:  0x300 (768)
        path            Path for DSP micro-code files.  Must be terminated
                        by a '\'.  Ex:
                            "c:\caap\dspfiles\"
                        If NULL then the current directory is used
 
        returns         NO_ERROR
                        ERROR             if something went wrong
                        FILE_NOT_FOUND    micro-code file not found
                        NOT_ENOUGH_MEMORY not enough memory to load micro-code
                        FILE_IO_ERROR     error reading micro-code file

Description
        Installs the dsp 96000 acquisition driver and initialises the 96000
        dsp board.
 #$%!i........................................................................*/

short dsp96_install (short instrument, short _io_addr, char *path)
{
	char filename[MAX_DOS_FULL_NAME+1];
	short hour, min, sec, hund;
	short ret;
	long irgb_time;
	long answer_len;
	time_t end_time;

	(void)instrument;
	if (uc_path == NULL)
		{
		uc_path = strdup(path==NULL ? "" : path);
		}
	bo_acquire_96000_time = 0.0;

	/* load 96002 micro-code */
	io_addr = _io_addr;
	strcpy (filename, path==NULL ? "" : path);
	strcat (filename, "sin_cos.bin");
	if (load_seq32 (TRUE, io_addr, filename) != NO_ERROR)
		{
		return (FILE_IO_ERROR);
		}
	strcpy (filename, path==NULL ? "" : path);
	if ((instrument & 127) == MB200)
		{
		strcat (filename, "coamb200.bin");
		encode_status = encode_mb200;
		decode_status = decode_mb200;
		i_mb200 = 1;
		}
	else if ((instrument & 127) == MB)
		{
		strcat (filename, "coamb100.bin");
		encode_status = encode_mb100;
		decode_status = decode_mb100;
		i_mb200 = 0;
		}

	if (load_seq32 (TRUE, io_addr, filename) != NO_ERROR)
		{
		return (FILE_IO_ERROR);
		}
	outport (io_addr, 0);
	/* init IRGB to time since midnight */
	cmd[0] = CFLG_IRGB;
	if (seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF) != NO_ERROR)
		{
		return(ERROR);
		}

	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
		}

	bo_gettime (&hour, &min, &sec, &hund);
	irgb_time = 100L * (360000L*hour + 6000*min + 100*sec + hund);
	inportb (io_addr);
	outport (io_addr+2, (word)(irgb_time&0xffff));
	outport (io_addr+2, (word)(irgb_time >> 16));
	outport (io_addr, HST_PC1);
	
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (inport (io_addr) & HST_M1)
		{
		if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
		}

	outport(io_addr, 0);

	/* read instrument status */
	cmd[0] = CFLG_STAT;
	if (seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF) != NO_ERROR)
		{
		return (ERROR_ACQ);
		}

	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
		}
	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		return (ERROR);
		}
	decode_status (status.status, &setup);

	/* setup operating conditions */
	setup.a.delay = det_delays[0];
	setup.b.delay = det_delays[1];
	encode_status (setup, cmd);
	if (seq32_tx_data (cmd, 6, LOAD_X, 0) != NO_ERROR)
		{
		return (ERROR);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
		}
	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		return (ERROR);
		}

	return (NO_ERROR);
}

/*#$%i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_RESET
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "seq32_pc.h"

        short dsp96_reset ()
 
        returns         NO_ERROR
                        ERROR             if something went wrong
                        FILE_NOT_FOUND    micro-code file not found
                        NOT_ENOUGH_MEMORY not enough memory to load micro-code
                        FILE_IO_ERROR     error reading micro-code file

Description
        Reset the dsp 96000 acquisition driver and initialises the 96000
        dsp board.
 #$%!i........................................................................*/

short dsp96_reset ()
{
	bo_acquire_96000_time = 0.0;
	return(dsp96_install (0, io_addr, uc_path));
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_REMOVE
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        void dsp96_remove(void)
 
Description
        Unloads the 96000 acquisition driver. This function should always
        be called before exiting a program that has used dsp96_install() to
        load the acquisition driver.
 #$%!i........................................................................*/

void dsp96_remove(void)
{
	bo_acquire_96000_time = 0.0;
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_WAIT_END_COAD
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        short dsp96_wait_end_coad(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r,
                                  YDATA *phs_i, double *acq_time,
                                  short (*userwait)(word scans_0, word scans_1,
                                                    word bad_scans,
                                                    long acq_num))
 
        spc_r           Structure of the collected data (or real spectrum)
        spc_i           Structure of the imaginary spectrum
        phs_r           Structure of the real phase spectrum
        phs_i           Structure of the imaginary spectrum 
        acq_time        Time at end of acquisition
        userwait()      User function, if NULL no fonction is called.
                        It is called repeatedly while waiting for the end of
                        the current acquisition.
                        The fonctions is called with the following parameters:
            scans_0     Direction 0 scan counter
            scans_1     Direction 1 scan counter
            bad_scans   Bad scan counter
            acq_num     Sequence number in multi-mode

            Returns     = 0:    
                            NO_ERROR          Continue
                       != 0:
                                              Abort acquisition and return
                                              error code 

        Returns         = 0:
                            NO_ERROR          Everything is ok
                       != 0:
                            ERROR_ACQ         Error during acquisition
                            TIMEOUT           Too much time between valid data
                            ...
Description
        Wait until the acquisition is completed. This function should be used
        when the acquisition has been started with dsp96_get_spec() or
        dsp96_get_int() with the "wait" parameter equal FALSE. Upon return, the
        buffers returned by the above mentioned function contains valid data.
#$%!i........................................................................*/

short dsp96_wait_end_coad(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r,
						YDATA *phs_i, double *acq_time,
						short (*userwait)(word scans_0, word scans_1,
										  word bad_scans, long acq_num))
{
	long scn_num_old = 0;
	long acq_num_old = 0;
	time_t end_time;
	float HPTR *tbuf;
	short ret;
	short m;
	long answer_len;
	short year, month, day;

	bo_timer_start (&end_time, TIMEOUT_DELAY);
	do
		{
		/* read status directly from memory */
		if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
			{
			goto no_stat;
			}
		if (userwait != NULL)
			{
			bo_acquire_96000_time = max(0.0,
				(status.delay-(status.irgb-status.irgb_start))/10000.0);
			if (status.scans_0 || status.scans_1) bo_acquire_96000_time = 0.0;

			ret = userwait ((word)status.scans_0, (word)status.scans_1,
							(word)status.scans_bad, status.seq_ctr);
			if (ret != NO_ERROR)
				{
				dsp96_hard_abort ();
				return (ret);
				}
			}
		bo_wait(0.1f);
		if (status.seq_ctr != acq_num_old ||
			status.scans_0+status.scans_1 != (long)scn_num_old)
			{
			bo_timer_start (&end_time, TIMEOUT_DELAY);
			acq_num_old = status.seq_ctr;
			scn_num_old = status.scans_0 + status.scans_1;
			}
no_stat:
		if (bo_timer_get (&end_time))
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		} while (!(inport (io_addr) & HST_M2) );
	bo_acquire_96000_time = 0.0;

	if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
		{
		dsp96_reset ();
		return (ERROR_ACQ);
		}
	decode_status (status.status, &setup);

	/* acquisition completed, send EOA flags to DSP */
	cmd[0] = 0;
	switch (acq_type)
		{
		case 2:	/* phase corrected spectrum */
			cmd[0] |= CFLG_PHASE;
		case 1:	/* complex spectrum */
			cmd[0] |= CFLG_CSPEC; 
		case 0: /* interferogram */
			cmd[0] |= CFLG_EOA | CFLG_CHANNEL_A;
		}
	if (seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF) != NO_ERROR)
		{
		dsp96_reset ();
		return (ERROR_ACQ);
		}

	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
		}

	/* get spectrum header */
	ret = seq32_get_data (&header, SCAN_HDR_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != SCAN_HDR_LEN)
		{
		dsp96_reset ();
		return (ERROR_ACQ);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
		}

	/* time and other parameters at end of acquisition run */
	bo_getdate (&year, &month, &day);
	*acq_time = bo_get_time_t (year, month, day, 0,0,0) + header.irgb/10000.0;

	m = scans ? 2 : 1; /* buffers are half size in 0 scan mode */
	switch(acq_type)
		{
		case 0: /* interferogram */
			if ((spc_r->buffer = (float *)bo_alloc (m*size[0])) == NULL)
				{
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}

			spc_r->npts = npts[0];
			spc_r->firstx = 0.0;
			spc_r->lastx = (bo_flaser/2.0);

			ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*spc_r);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			if (scans)
				{
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
					}
				ret	= seq32_get_data (spc_r->buffer+spc_r->npts, spc_r->npts,
									  &answer_len);
				if (ret != NO_ERROR || answer_len != spc_r->npts)
					{
					free_ydata (*spc_r);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				}
			break;
		case 1: /* complex spectrum */
			if ((phs_r->buffer = (float *)bo_alloc (m*p_size[0])) == NULL)
				{
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			if ((phs_i->buffer = (float *)bo_alloc (m*p_size[0])) == NULL)
				{
				free_ydata (*phs_r);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			if ((spc_r->buffer = (float *)bo_alloc (m*size[0])) == NULL)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			if ((spc_i->buffer = (float *)bo_alloc (m*size[0])) == NULL)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}

			phs_r->firstx = phs_i->firstx = p_firstx[0];
			phs_r->lastx  = phs_i->lastx  = p_lastx[0];
			phs_r->npts   = phs_i->npts   = p_npts[0];
			spc_r->firstx = spc_i->firstx = firstx[0];
			spc_r->lastx  = spc_i->lastx  = lastx[0];
			spc_r->npts   = spc_i->npts   = npts[0];

			/* phase no longer available */

			ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				free_ydata (*spc_i);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			// wait for response with a timeout
			bo_timer_start(&end_time, TIMEOUT_DELAY);
			while (!(inport (io_addr) & HST_M1) )
				{
				if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
				}
			ret	= seq32_get_data (spc_i->buffer, spc_i->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_i->npts)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				free_ydata (*spc_i);
				dsp96_reset ();
				return (ERROR_ACQ);
				}

			if (scans)
				{
				/* phase no longer available */

				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
					}
				ret	= seq32_get_data (spc_r->buffer+spc_r->npts, spc_r->npts,
									  &answer_len);
				if (ret != NO_ERROR || answer_len != spc_r->npts)
					{
					free_ydata (*phs_r);
					free_ydata (*phs_i);
					free_ydata (*spc_r);
					free_ydata (*spc_i);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
					}
				ret	= seq32_get_data (spc_i->buffer+spc_i->npts, spc_i->npts,
									  &answer_len);
				if (ret != NO_ERROR || answer_len != spc_i->npts)
					{
					free_ydata (*phs_r);
					free_ydata (*phs_i);
					free_ydata (*spc_r);
					free_ydata (*spc_i);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				}
			break;
		case 2: /* spectrum */
			if ((spc_r->buffer = (float *)bo_alloc (size[0])) == NULL)
				{
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			spc_r->firstx = firstx[0];
			spc_r->lastx  = lastx[0];
			spc_r->npts   = npts[0];

			ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*spc_r);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			if (scans)
				{
				if ((tbuf = (float *)bo_alloc (size[0])) == NULL)
					{
					free_ydata (*spc_r);
					dsp96_reset ();
					return (NOT_ENOUGH_MEMORY);
					}
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
					}
				ret	= seq32_get_data (tbuf, spc_r->npts, &answer_len);
				if (ret != NO_ERROR || answer_len != spc_r->npts)
					{
					free_ydata (*spc_r);
					bo_free (tbuf);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				v_add (tbuf, spc_r->buffer, spc_r->buffer, spc_r->npts);
				v_scale (spc_r->buffer, 0.5f, spc_r->buffer, spc_r->npts);
				bo_free (tbuf);
				}	
			break;
		}

	return(NO_ERROR);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_GET_COAD
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        short dsp96_get_coad(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r,
                                  YDATA *phs_i, double *acq_time,
                                  long *acq_num)
 
        spc_r           Structure of the collected data (or real spectrum)
        spc_i           Structure of the imaginary spectrum
        phs_r           Structure of the real phase spectrum
        phs_i           Structure of the imaginary spectrum 
        acq_time        Time at end of acquisition
        acq_num         Sequence number in multi-mode

        Returns         = 0:
                            NO_ERROR          Everything is ok
                       != 0:
                            ERROR_ACQ         Error during acquisition
                            TIMEOUT           Too much time between valid data
                            ...
Description
        This function should be called repeatedly after dsp96_wait_end_coad()
        to retrieve the rest of the subfiles from a multi_acquisition. It can
        also be used after dsp96_get_spec(), dsp96_get_raw_spec(),
        dsp96_get_int() or dsp96_get_raw_int() if the wait parameter in TRUE.
#$%!i........................................................................*/

short dsp96_get_coad(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r, YDATA *phs_i,
					double *acq_time, long *acq_num)
{
	short det_indx;
	float HPTR *tbuf;
	short ret, m;
	short year, month, day;
	long answer_len;
	time_t end_time;

	/* compute where we are in the sequence */
	if (++cur_acq_num >= status.op_cond.n_seq * num_det) return(ERROR);
	det_indx = cur_acq_num >= status.op_cond.n_seq;
	*acq_num = cur_acq_num % status.op_cond.n_seq;

	/* setup for second channel */
	if (cur_acq_num == status.op_cond.n_seq) /* 1st block, 2nd detector	*/
		{
		status.op_cond.spec_1st = spec2_1st;
		status.op_cond.spec_len	= spec2_len;
		status.op_cond.cflags = 0;
		switch (acq_type)
			{
			case 2:	/* phase corrected spectrum */
				status.op_cond.cflags |= CFLG_PHASE;
			case 1:	/* complex spectrum */
				status.op_cond.cflags |= CFLG_CSPEC; 
			case 0: /* interferogram */
				status.op_cond.cflags |= CFLG_EOA;
			}
		ret = seq32_tx_data (&status.op_cond, 10, LOAD_X, OP_COND_OFF);
		if (ret != NO_ERROR)
			{
			dsp96_reset ();
			return (ERROR_ACQ);
			}
		// wait for response with a timeout
		bo_timer_start (&end_time, TIMEOUT_DELAY);
		while (!(inport (io_addr) & HST_M1) )
			{
			if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
			}
		}

	/* get spectrum header */
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
		}
	ret = seq32_get_data (&header, SCAN_HDR_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != SCAN_HDR_LEN)
		{
		dsp96_reset ();
		return (ERROR_ACQ);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
		}
	bo_getdate (&year, &month, &day);
	*acq_time = bo_get_time_t (year, month, day, 0,0,0) + header.irgb/10000.0;

	m = scans ? 2 : 1; /* buffers are half size in 0 scan mode */
	switch (acq_type)
		{
		case 0: /* interferogram */
			if ((spc_r->buffer = (float *)bo_alloc (m*size[0])) == NULL)
				{
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}

			spc_r->npts = npts[0];
			spc_r->firstx = 0.0;
			spc_r->lastx = (bo_flaser/2.0);

			ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*spc_r);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			if (scans)
				{
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
					}
				ret	= seq32_get_data (spc_r->buffer+spc_r->npts, spc_r->npts,
									  &answer_len);
				if (ret != NO_ERROR || answer_len != spc_r->npts)
					{
					free_ydata (*spc_r);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				}
			break;
		case 1: /* complex spectrum */
			if ((phs_r->buffer = (float *) bo_alloc (m*p_size[det_indx])) ==
				NULL)
				{
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			if ((phs_i->buffer = (float *)bo_alloc (m*p_size[det_indx])) ==
				NULL)
				{
				free_ydata (*phs_r);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			if ((spc_r->buffer = (float *)bo_alloc (m*size[det_indx])) ==
				NULL)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			if ((spc_i->buffer = (float *)bo_alloc (m*size[det_indx])) ==
				NULL)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}

			phs_r->firstx = phs_i->firstx = p_firstx[det_indx];
			phs_r->lastx  = phs_i->lastx  = p_lastx[det_indx];
			phs_r->npts   = phs_i->npts   = p_npts[det_indx];
			spc_r->firstx = spc_i->firstx = firstx[det_indx];
			spc_r->lastx  = spc_i->lastx  = lastx[det_indx];
			spc_r->npts   = spc_i->npts   = npts[det_indx];

			/* phase no longer available */

			ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				free_ydata (*spc_i);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			// wait for response with a timeout
			bo_timer_start(&end_time, TIMEOUT_DELAY);
			while (!(inport (io_addr) & HST_M1) )
				{
				if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
				}
			ret	= seq32_get_data (spc_i->buffer, spc_i->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_i->npts)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				free_ydata (*spc_i);
				dsp96_reset ();
				return (ERROR_ACQ);
				}

			if (scans)
				{
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
					}
				ret	= seq32_get_data (spc_r->buffer+spc_r->npts,
									  spc_r->npts, &answer_len);
				if (ret != NO_ERROR || answer_len != spc_r->npts)
					{
					free_ydata (*phs_r);
					free_ydata (*phs_i);
					free_ydata (*spc_r);
					free_ydata (*spc_i);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
					}
				ret	= seq32_get_data (spc_i->buffer+spc_i->npts,
									  spc_i->npts, &answer_len);
				if (ret != NO_ERROR || answer_len != spc_i->npts)
					{
					free_ydata (*phs_r);
					free_ydata (*phs_i);
					free_ydata (*spc_r);
					free_ydata (*spc_i);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				}
			break;
		case 2: /* spectrum */
			if ((spc_r->buffer = (float *)bo_alloc (size[det_indx])) == NULL)
				{
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			spc_r->firstx = firstx[det_indx];
			spc_r->lastx  = lastx[det_indx];
			spc_r->npts   = npts[det_indx];

			ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*spc_r);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			if (scans)
				{
				if ((tbuf = (float *)bo_alloc (size[det_indx])) == NULL)
					{
					free_ydata (*spc_r);
					dsp96_reset ();
					return (NOT_ENOUGH_MEMORY);
					}
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE) return (TIMEOUT);
					}
				ret	= seq32_get_data (tbuf, spc_r->npts, &answer_len);
				if (ret != NO_ERROR || answer_len != spc_r->npts)
					{
					bo_free (tbuf);
					free_ydata (*spc_r);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				v_add(tbuf, spc_r->buffer, spc_r->buffer, spc_r->npts);
				v_scale (spc_r->buffer, 0.5f, spc_r->buffer, spc_r->npts);
				bo_free (tbuf);
				}	
			break;
		}

	return(NO_ERROR);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_HARD_ABORT
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        short dsp96_hard_abort (void)

        Returns         NO_ERROR
                        ERROR_ACQ
                        ERROR_NO_ACQ

Description
        Kill current acquisition without preserving the data. The acquisition
        can then be started again normally.

See also
        dsp96_remove()
 #$%!i........................................................................*/

short dsp96_hard_abort (void)
{
	bo_acquire_96000_time = 0.0;

	cmd[0] = CFLG_SOFT_ABORT;
	if (seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF) != NO_ERROR)
		{
		dsp96_reset();
		return(ERROR_ACQ);
		}
	return(NO_ERROR);
}

/*#$%!i**************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_SPEED
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        void dsp96_speed (float *speed);
 
        speed			Speed in scans per minute

Description
        Retrieves the instrument speed on non MB200 instruments.
 #$%!i........................................................................*/

void dsp96_speed (float *speed)
{
	// patched to get 2 extra words	at end of status, these word are for IRGB
	if (seq32_rx_data (&status, MB_STATUS_LEN+2, DUMP_X, 0) != NO_ERROR)
		{
		return;
		}

	// get current speed
	*speed = 300000.0 / (status.data_max - status.data_min);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_STAT
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        short dsp96_stat (word *scans_0, word *scans_1, word *scans_bad,
                        long *acq_num);
 
        scans_0         Direction 0 scan counter
        scans_1         Direction 1 scan counter
        scans_bad       Bad scan counter
        acq_num         Sequence number
 
        Returns         NO_ERROR
                        ERROR_NO_ACQ   (status not yet available)
                        ERROR_ACQ      (status invalid (has just changed)

Description
        Retrieves the state of the current acquisition. The values
        returned by dsp96_stat() are not valid when an error code is returned
 #$%!i........................................................................*/

short dsp96_stat (word *scans_0, word *scans_1, word *scans_bad,
				  long *acq_num)
{
	if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
		{
		return (ERROR_ACQ);
		}

	*scans_0   = (word)status.scans_0;
	*scans_1   = (word)status.scans_1;
	*scans_bad = (word)status.scans_bad;
	*acq_num   = status.seq_ctr;

	bo_acquire_96000_time = max(0.0,
		(status.delay-(status.irgb-status.irgb_start))/10000.0);
	if (status.scans_0 || status.scans_1) bo_acquire_96000_time = 0.0;

	return (NO_ERROR);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_STATUS
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        short dsp96_status(word *scans_0, word *scans_1, word *scans_bad,
                           long *acq_num, short *resolution, short *speed,
                           short det1[4], short det2[4], short *acq_err);
 
        scans_0         Direction 0 scan counter
        scans_1         Direction 1 scan counter
        scans_bad       Bad scan counter
        acq_num         Sequence number
        resolution      Resolution
                               0 = 128cm-1
                               1 =  64cm-1
                               2 =  32cm-1
                               3 =  16cm-1
                               4 =   8cm-1
                               5 =   4cm-1
                               6 =   2cm-1
                               7 =   1cm-1
        speed           Scanning speed, for now always 2 for very fast
        det1            Detector 1 status
                               det1[0] = detector ID code (-1=no detector)
                               det1[1] = oversampling (0=1 sample, 1=2 samples
                                                       per fringe)
                               det1[2] = first stage gain
                                          currently always 1
                                          becomes -1 when detector is saturated
                               det1[3] = second stage gain
                                          becomes -gain if amplifier saturated
        det2            Detector 2 status
                               det2[0] = detector ID code (-1=no detector)
                               det2[1] = oversampling (0=1 sample, 1=2 samples
                                                       per fringe)
                               det2[2] = first stage gain
                                          currently always 1
                                          becomes -1 when detector is saturated
                               det2[3] = second stage gain
                                          becomes -gain if amplifier saturated
        acq_err         Last acquisition error code
 
        Returns         NO_ERROR
                        ERROR_NO_ACQ   (status not yet available)
                        ERROR_ACQ      (status invalid (has just changed)

Description
        Retrieves the state of the current acquisition. The values
        returned by dsp96_status() are not valid when an error code is returned.
 #$%!i........................................................................*/

short dsp96_status(word *scans_0, word *scans_1, word *scans_bad,
					long *acq_num, short *resolution, short *speed,
					short det1[4], short det2[4], short *acq_err)
{
	if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
		{
		return (ERROR_ACQ);
		}
	decode_status (status.status, &setup);

	bo_acquire_96000_time = max(0.0,
		(status.delay-(status.irgb-status.irgb_start))/10000.0);
	if (status.scans_0 || status.scans_1) bo_acquire_96000_time = 0.0;

	*scans_0   = (word)status.scans_0;
	*scans_1   = (word)status.scans_1;
	*scans_bad = (word)status.scans_bad;
	*acq_num   = status.seq_ctr;

	*resolution= (*(short *)&status.status & 7);
	if (i_mb200) *resolution = 7 - *resolution;

	*speed     = 2;

	det1[0]    = setup.a.present ? setup.a.id : -1;
	det1[1]    = setup.a.over;
    det1[2]    = setup.a.sat_1st ? -1 : 1;
    det1[3]    = setup.a.gain * (setup.a.sat_last ? -1 : 1);

	det2[0]    = setup.b.present ? setup.a.id : -1;
	det2[1]    = setup.b.over;
    det2[2]    = setup.b.sat_1st ? -1 : 1;
    det2[3]    = setup.b.gain * (setup.a.sat_last ? -1 : 1);

	*acq_err   = NO_ERROR;

	return(NO_ERROR);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_SET_STATUS
File:   ACQ_96.C
Author: Thomas Buijs
Date:   October 1992

Synopsis
        #include "dsp96_pc.h"

        short dsp96_set_status(short resolution, short speed, short det1[4],
                               short det2[4], short (*userwait) (word,
																word, word));

        resolution      Resolution
                               0 = 128cm-1
                               1 =  64cm-1
                               2 =  32cm-1
                               3 =  16cm-1
                               4 =   8cm-1
                               5 =   4cm-1
                               6 =   2cm-1
                               7 =   1cm-1
        speed           Scanning speed, for now always 2 for very fast
        det1            Detector 1 status
                               det1[0] = not used
                               det1[1] = oversampling (0=1 sample, 1=2 samples
                                                       per fringe)
                               det1[2] = not used
                               det1[3] = second stage gain (must be 1,2,4,8,16)
        det2            Detector 2 status
                               det2[0] = not used
                               det2[1] = oversampling (0=1 sample, 1=2 samples
                                                       per fringe)
                               det2[2] = not used
                               det2[3] = second stage gain (must be 1,2,4,8,16)
		short userwait () Function that is called repeatedly while waiting
						  for the resolution to change. can be NULL for no
						  function.
						  It is called with the following parameters:
			  scans_0     Direction 0 scan counter
			  scans_1     Direction 1 scan counter
			  bad_scans   Bad scan counter
			  acq_num     coad sequence number if more than 1 coad requested


				return 0: function continues waiting
				return negative error code: function aborts and returns error
				                            code

        Returns         NO_ERROR
                        ERROR_SETUP  hardware setup failled
Description
        Update the spectrometer status.
 #$%!i........................................................................*/

short dsp96_set_status(short resolution, short speed, short det1[4],
		       short det2[4],
			   short (*userwait)(word scans_0, word scans_1,
			   			 		 word scans_bad ))
{
	short ret;
	long answer_len;
	short default_det[4] = {0, 1, 1, 1};
	time_t end_time;

    /* If det1 and/or det2 are NULL use default */
    if ( det1 == NULL || det2 == NULL )
    	{
        det1 = det2 = default_det;
        }

	(void)speed;
    (void*)userwait; /* Not implemented yet */

	if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return (ERROR_SETUP);
		}
	decode_status (status.status, &setup);

	setup.resolution = 1 << (7-resolution);
	setup.a.gain = det1[3];
	setup.b.gain = det2[3];
	setup.a.delay = det_delays[0];
	setup.b.delay = det_delays[1];

	encode_status (setup, cmd);

	if (seq32_tx_data (cmd, 6, LOAD_X, 0) != NO_ERROR)
		{
		dsp96_reset ();
		return (ERROR_SETUP);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}
	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		dsp96_reset();
		return (ERROR);
		}

	decode_status (status.status, &setup);
	setup.a.delay = det_delays[0];
	setup.b.delay = det_delays[1];
	encode_status (setup, cmd);

	if (seq32_tx_data (cmd, 6, LOAD_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return(ERROR_SETUP);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}
	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		dsp96_reset();
		return (ERROR);
		}
	return(NO_ERROR);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_GET_INT
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        short dsp96_get_int(YDATA *interf, word nbr_scans, long nbr_acq,
                            double delay, char wait, double *acq_time,
                            short (*userwait)(word scans_0, word scans_1,
                                              word scans_bad, long acq_num))
 
        interf          Pointer to the collected interferogram data structure
        nbr_scans       Number of scan requested ( one scan is defined as a 
                                                   forward AND reverse strokes)
                        If 0 ==> "half scan" (forward OR reverse)
        nbr_acq         Number of coad sequences requested
        wait            FALSE -> Start acquisition and leave it in background. 
                                 Use dsp96_wait_end_coadd() to get data
                        TRUE  -> Start and wait for end of acquisition

        acq_time        Time of end of acquistion, valid only if wait is TRUE

        userwait()      User function, if NULL no fonction is called.
                        It is called repeatedly while waiting for the end of
                        the current acquisition only if wait is TRUE.
                        The fonctions is called with the following parameters:
            scans_0     Direction 0 scan counter
            scans_1     Direction 1 scan counter
            bad_scans   Bad scan counter
            acq_num     Sequence number

            Returns     = 0:    
                            NO_ERROR          Continue
                       != 0:
                                              Abort acquisition and return
                                              error code 

        Returns         = 0:
                            NO_ERROR          Everything is ok
                        < 0:
                            TIMEOUT           Too much time between valid data
                            ERROR_SETUP       Error during setup
                            NOT_ENOUGH_MEMORY Not enough memory
                            ERROR_START       Error during start
                            ERROR_ACQ         Error during acquisition

Description
        Get one or more coadded interferograms from the interferometer using
        the DSP96000 board. It must be called after dsp96_install(). The
        interferogram data is normalized in volts, and it will never be larger
        than 5. The forward and reverse part of the interferogram will be
        coadded separetly and appended one after the other.

Cautions
        Do not forget to release the interferogram buffer with bo_free() when 
        it is no longer in use.  If "wait" is set to FALSE, never attempt to
        manipulate data in "interf" before calling dsp96_wait_end_coadd()
        otherwise unpredictable results could occur.
#$%!i........................................................................*/

short dsp96_get_int (YDATA *interf, word nbr_scans, long nbr_acq,
					 double delay, char wait, double *acq_time,
					 short (*userwait)(word scans_0, word scans_1,
									   word scans_bad, long acq_num))
{
	long scn_num_old = 0;
	long acq_num_old = 0;
	time_t end_time;
	short ret, m;
	short year, month, day;
	long answer_len;
	Op_cond op_cond;

	if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return (ERROR_SETUP);
		}

	decode_status (status.status, &setup);
	setup.a.delay = det_delays[0];
	setup.b.delay = det_delays[1];
	encode_status (setup, cmd);

	if (seq32_tx_data (cmd, 6, LOAD_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return(ERROR_SETUP);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}
	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		dsp96_reset();
		return (ERROR);
		}

	scans = nbr_scans;
	cur_acq_num = 0L;

	op_cond.interf_scale = 2.5 / 32768.0f;

	op_cond.n_seq      = nbr_acq;
	op_cond.n_scans    = nbr_scans;
	op_cond.coad_delay = (long)(delay*10000);
	op_cond.cflags     = CFLG_OP | CFLG_CHANNEL_A | CFLG_DIR_0;
	acq_type = 0;

	if (seq32_tx_data (&op_cond, 10, LOAD_X, OP_COND_OFF) != NO_ERROR)
		{
		dsp96_reset ();
		return (ERROR_START);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}

	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		dsp96_reset ();
		return (ERROR_START);
		}
	outport (io_addr, inport (io_addr) | HST_PC2);
	decode_status (status.status, &setup);

	npts[0] = npts[1] = 32768L / setup.resolution;
	size[0] = size[1] = npts[0] * sizeof (float);

	num_det = (!!setup.a.present) + (!!setup.b.present);
	if (num_det == 0)
		{
		dsp96_reset ();
		return (ERROR_ACQ);
		}
	if (wait)
		{
		bo_timer_start (&end_time, TIMEOUT_DELAY);
		do
			{
			/* read status directly from memory */
			if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
				{
				goto no_stat;
				}
			if (userwait != NULL)
				{
				bo_acquire_96000_time =
					(status.delay-(status.irgb-status.irgb_start))/10000.0;
				if (status.scans_0 || status.scans_1)
					bo_acquire_96000_time = 0.0;

				ret = userwait ((word)status.scans_0, (word)status.scans_1,
								(word)status.scans_bad, status.seq_ctr);
				if (ret != NO_ERROR)
					{
					dsp96_hard_abort ();
					return (ret);
					}
				}
			bo_wait(0.1f);
			if (status.seq_ctr != acq_num_old ||
				status.scans_0+status.scans_1 != (long)scn_num_old)
				{
				bo_timer_start (&end_time, TIMEOUT_DELAY);
				acq_num_old = status.seq_ctr;
				scn_num_old = status.scans_0 + status.scans_1;
				}
no_stat:
			if (bo_timer_get (&end_time) == TRUE)
				{
				dsp96_reset ();
				return (TIMEOUT);
				}
			} while (!(inport (io_addr) & HST_M2) );

		bo_acquire_96000_time = 0.0;

		if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
			{
			dsp96_reset ();
			return (ERROR_START);
			}
		decode_status (status.status, &setup);

		/* acquisition completed, send EOA flags to DSP */
		cmd[0] = CFLG_EOA;
		if (seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF) != NO_ERROR)
			{
			dsp96_reset ();
			return (ERROR_ACQ);
			}
		while (!(inport (io_addr) & HST_M1) )
			{
			}

		/* get spectrum header */
		ret = seq32_get_data (&header, SCAN_HDR_LEN, &answer_len);
		if (ret != NO_ERROR || answer_len != SCAN_HDR_LEN)
			{
			dsp96_reset ();
			return (ERROR_ACQ);
			}

		/* time and other parameters at end of acquisition run */
		bo_getdate (&year, &month, &day);
		*acq_time = bo_get_time_t (year, month, day, 0,0,0) +
							header.irgb/10000.0;

		m = scans ? 2 : 1; /* buffers are half size in 0 scan mode */
		if ((interf->buffer = (float *)bo_alloc (m*size[0])) == NULL)
			{
			dsp96_reset ();
			return (NOT_ENOUGH_MEMORY);
			}

		interf->npts = npts[0];
		interf->firstx = 0.0;
		interf->lastx = (bo_flaser/2.0);

		// wait for response with a timeout
		bo_timer_start(&end_time, TIMEOUT_DELAY);
		while (!(inport (io_addr) & HST_M1) )
			{
			if (bo_timer_get (&end_time) == TRUE)
				{
				dsp96_reset ();
				return (TIMEOUT);
				}
			}
		ret	= seq32_get_data (interf->buffer, interf->npts, &answer_len);
		if (ret != NO_ERROR || answer_len != interf->npts)
			{
			free_ydata (*interf);
			dsp96_reset ();
			return (ERROR_ACQ);
			}
		if (scans)
			{
			// wait for response with a timeout
			bo_timer_start(&end_time, TIMEOUT_DELAY);
			while (!(inport (io_addr) & HST_M1) )
				{
				if (bo_timer_get (&end_time) == TRUE)
					{
					dsp96_reset ();
					return (TIMEOUT);
					}
				}
			ret	= seq32_get_data (interf->buffer+interf->npts, interf->npts,
								  &answer_len);
			if (ret != NO_ERROR || answer_len != interf->npts)
				{
				free_ydata (*interf);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			}
		}

	return(NO_ERROR);	
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_GET_SPEC
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "spectrum.h"
        #include "dsp96_pc.h"

        short dsp96_get_spec (YDATA *spec, word nbr_scans, long nbr_acq,
                              double delay, float user_sna, float user_sxa,
							  float user_snb, float user_sxb,
							  short apodization, char wait, double *acq_time,
                              short (*userwait) (word scans_0, word scans_1,
                                               word scans_bad, long acq_num))
 
        spec            Structure of the collected data
        nbr_scans       Number of scan requested ( one scan is defined as a
                                                   forward AND reverse strokes)
                        If 0 ==> "half scan" (forward OR reverse)
        nbr_acq         Number of coad sequences requested
        user_sna        User requested sigma minimum in cm-1 for detector a
        user_sxa        User requested sigma maximum in cm-1 for detector a
        user_snb        User requested sigma minimum in cm-1 for detector b
        user_sxb        User requested sigma maximum in cm-1 for detector b
        apodization     Apodization type to be used for spectrum calculation.
                        The apodization types are:
                                BOXCAR
                                BARTLET
                                COSINE (recommended)
                                HAMMING
                                BLACKMAN
                                GAUSSIAN
                                NORTON_B_WEAK
                                NORTON_B_MEDIUM
                                NORTON_B_STRONG
        wait            FALSE -> Start acquisition and leave it in background.
                                 Use acq_wait_end_coadd() to get data
                        TRUE  -> Start and wait for end of acquisition 
        acq_time        Time of end of acquisition, valid only if wait is TRUE
                        if NULL, don't care
        userwait()      User function, if NULL no fonction is called.
                        It is called repeatedly while waiting for the end of
                        the current acquisition only if wait is TRUE.
                        The fonctions is called with the following parameters:
            scans_0     Direction 0 scan counter
            scans_1     Direction 1 scan counter
            bad_scans   Bad scan counter
            acq_num     Sequene number

            Returns     = 0:    
                            NO_ERROR          Continue
                       != 0:
                                              Abort acquisition and return
                                              error code 
        Returns         = 0:
                            NO_ERROR          Everything is ok
                       != 0:
                            ERROR_SETUP       Error during setup
                            NOT_ENOUGH_MEMORY Not enough memory
                            ERROR_START       Error during start
                            ERROR_ACQ         Error during acquisition
                            TIMEOUT           Too much time between valid data
                            ...
Description
        Get one or more coadded, apodized, phase corrected spectra from the
        interferometer.
        It must be called after dsp96_install(). 
        To do background acquisition, the "wait" parameter should be FALSE
        and dsp96_wait_end_coad() should be called to grab the collected data.

Cautions
        Do not forget to release the spectrum buffer with bo_free() when
        it is no longer in use. If "wait" is set to FALSE, never attempt to
        manipulate data in "spec" before calling dsp96_wait_end_coadd()
        otherwise unpredictable results could occur.
#$%!i........................................................................*/

short dsp96_get_spec (YDATA *spec, word nbr_scans, long nbr_acq, double delay,
						float user_sna, float user_sxa, float user_snb,
						float user_sxb, short apodization, char wait,
						double *acq_time,
						short (*userwait) (word scans_0, word scans_1,
											word bad_scans, long acq_num))
{
	long scn_num_old = 0;
	long acq_num_old = 0;
	time_t end_time;
	long s1, s2;
	long double tmp;
	float HPTR *temp;
	short ret;
	short year, month, day;
	long answer_len;
	Op_cond op_cond;

	if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return (ERROR_SETUP);
		}

	decode_status (status.status, &setup);
	setup.a.delay = det_delays[0];
	setup.b.delay = det_delays[1];
	encode_status (setup, cmd);

	if (seq32_tx_data (cmd, 6, LOAD_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return(ERROR_SETUP);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}
	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		dsp96_reset();
		return (ERROR);
		}

	scans = nbr_scans;
	cur_acq_num = 0L;

	if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return (ERROR_SETUP);
		}
	decode_status (status.status, &setup);

	npts[0] = npts[1] = 16384L / setup.resolution;

	tmp = (2 * npts[0]) / bo_flaser;
	s1  = (long)(user_sna * tmp);
	s2  = (long)(user_sxa * tmp + 0.99f);
	if (s1 >= npts[0]) s1 = 0;
	if (s2 >= npts[0]) s2 = npts[0]-1;
	firstx[0] = (double)(s1 / tmp);
	lastx[0]  = (double)(s2 / tmp);
	npts[0]   = s2 - s1 + 1;

	/* must be here because s1 is overwritten */
	op_cond.spec_1st   = s1;

	tmp = (2 * npts[1]) / bo_flaser;
	s1  = (long)(user_snb * tmp);
	s2  = (long)(user_sxb * tmp + 0.99f);
	if (s1 >= npts[1]) s1 = 0;
	if (s2 >= npts[1]) s2 = npts[1]-1;
	firstx[1] = (double)(s1 / tmp);
	lastx[1]  = (double)(s2 / tmp);
	npts[1]   = s2 - s1 + 1;

	size[0] = npts[0] * sizeof (float);
	size[1] = npts[1] * sizeof (float);

	op_cond.interf_scale = 2.5 / 32768.0f;

	op_cond.n_seq      = nbr_acq;
	op_cond.n_scans    = nbr_scans;
	op_cond.coad_delay = (long)(delay*10000);
	op_cond.spec_len   = npts[0];
	spec2_1st          = s1;
	spec2_len          = npts[1];
	op_cond.spec_apod  = apodization;
	op_cond.phase_len  = 128;
	op_cond.phase_apod = GAUSSIAN;
	op_cond.cflags     = CFLG_OP | CFLG_CHANNEL_A | CFLG_DIR_0;
	acq_type = 2;

	if (seq32_tx_data (&op_cond, 10, LOAD_X, OP_COND_OFF) != NO_ERROR)
		{
		dsp96_reset ();
		return (ERROR_START);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}

	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		dsp96_reset ();
		return (ERROR_START);
		}
	outport (io_addr, inport (io_addr) | HST_PC2);
	decode_status (status.status, &setup);

	num_det = (!!setup.a.present) + (!!setup.b.present);
	if (num_det == 0)
		{
		dsp96_reset ();
		return (ERROR_ACQ);
		}

	if (wait)
		{
		bo_timer_start (&end_time, TIMEOUT_DELAY);
		do
			{
			/* read status directly from memory */
			if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
				{
				goto no_stat;
				}
			if (userwait != NULL)
				{
				bo_acquire_96000_time =
					(status.delay-(status.irgb-status.irgb_start))/10000.0;
				if (status.scans_0 || status.scans_1)
					bo_acquire_96000_time = 0.0;

				ret = userwait ((word)status.scans_0, (word)status.scans_1,
								(word)status.scans_bad, status.seq_ctr);
				if (ret != NO_ERROR)
					{
					dsp96_hard_abort ();
					return (ret);
					}
				}
			bo_wait(0.1f);
			if (status.seq_ctr != acq_num_old ||
				status.scans_0+status.scans_1 != (long)scn_num_old)
				{
				bo_timer_start (&end_time, TIMEOUT_DELAY);
				acq_num_old = status.seq_ctr;
				scn_num_old = status.scans_0 + status.scans_1;
				}
no_stat:
			if (bo_timer_get (&end_time) == TRUE)
				{
				dsp96_reset ();
				return (TIMEOUT);
				}
			} while (!(inport (io_addr) & HST_M2) );

		bo_acquire_96000_time = 0.0;

		if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
			{
			dsp96_reset ();
			return (ERROR_START);
			}
		decode_status (status.status, &setup);

		/* acquisition completed, send EOA flags to DSP */
		cmd[0] = CFLG_EOA | CFLG_CSPEC | CFLG_PHASE;
		if (seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF) != NO_ERROR)
			{
			dsp96_reset ();
			return (ERROR_ACQ);
			}
		// wait for response with a timeout
		bo_timer_start(&end_time, TIMEOUT_DELAY);
		while (!(inport (io_addr) & HST_M1) )
			{
			if (bo_timer_get (&end_time) == TRUE)
				{
				dsp96_reset ();
				return (TIMEOUT);
				}
			}

		/* get spectrum header */
		ret = seq32_get_data (&header, SCAN_HDR_LEN, &answer_len);
		if (ret != NO_ERROR || answer_len != SCAN_HDR_LEN)
			{
			dsp96_reset ();
			return (ERROR_ACQ);
			}

		/* time and other parameters at end of acquisition run */
		bo_getdate (&year, &month, &day);
		*acq_time = bo_get_time_t (year, month, day, 0,0,0) +
								header.irgb/10000.0;

		if ((spec->buffer = (float *)bo_alloc (size[0])) == NULL)
			{
			dsp96_reset ();
			return (NOT_ENOUGH_MEMORY);
			}

		spec->firstx = firstx[0];
		spec->lastx  = lastx[0];
		spec->npts   = npts[0];

		// wait for response with a timeout
		bo_timer_start(&end_time, TIMEOUT_DELAY);
		while (!(inport (io_addr) & HST_M1) )
			{
			if (bo_timer_get (&end_time) == TRUE)
				{
				dsp96_reset ();
				return (TIMEOUT);
				}
			}
		ret	= seq32_get_data (spec->buffer, spec->npts, &answer_len);
		if (ret != NO_ERROR || answer_len != spec->npts)
			{
			free_ydata (*spec);
			dsp96_reset ();
			return (ERROR_ACQ);
			}

		if (scans)
			{
			if ((temp = (float *)bo_alloc (size[0])) == NULL)
				{
				free_ydata (*spec);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			// wait for response with a timeout
			bo_timer_start(&end_time, TIMEOUT_DELAY);
			while (!(inport (io_addr) & HST_M1) )
				{
				if (bo_timer_get (&end_time) == TRUE)
					{
					dsp96_reset ();
					return (TIMEOUT);
					}
				}
			ret	= seq32_get_data (temp, spec->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spec->npts)
				{
				free_ydata (*spec);
				bo_free (temp);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			v_add (spec->buffer, temp, spec->buffer, spec->npts);
			v_scale (spec->buffer, 0.5f, spec->buffer, spec->npts);
			bo_free (temp);
			}
		}

	return(NO_ERROR);	
}

/*#$%!i************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_GET_RAW_SPEC
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "spectrum.h"
        #include "dsp96_pc.h"

        short dsp96_get_raw_spec(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r,
                             YDATA *phs_i, word nbr_scans, long nbr_acq,
                             double delay, float user_sna, float user_sxa,
                             float user_snb, float user_sxb, short apodization,
                             char wait, double *acq_time,
                             short (*userwait)(word scans_0, word scans_1,
                                               word scans_bad, long acq_num))
 
        spc_r           Real spectrum
        spc_i           Imaginary spectrum
        phs_r           Real ohase spectrum
        phs_i           Imaginary phase spectrum
        nbr_scans       Number of scan requested ( one scan is defined as a
                                                   forward AND reverse strokes)
                        If 0 ==> "half scan" (forward OR reverse)
        nbr_acq         Number of coad sequences requested
        user_sna        User requested sigma minimum in cm-1 for detector a
        user_sxa        User requested sigma maximum in cm-1 for detector a
        user_snb        User requested sigma minimum in cm-1 for detector b
        user_sxb        User requested sigma maximum in cm-1 for detector b
        apodization     Apodization type to be used for spectrum calculation.
                        The apodization types are:
                                BOXCAR
                                BARTLET
                                COSINE (recommended)
                                HAMMING
                                BLACKMAN
                                GAUSSIAN
                                NORTON_B_WEAK
                                NORTON_B_MEDIUM
                                NORTON_B_STRONG
        wait            FALSE -> Start acquisition and leave it in background.
                                 Use acq_wait_end_coadd() to get data
                        TRUE  -> Start and wait for end of acquisition 
        acq_time        Time of end of acquisition, valid only if wait is TRUE
                        if NULL, don't care
        userwait()      User function, if NULL no fonction is called.
                        It is called repeatedly while waiting for the end of
                        the current acquisition only if wait is TRUE.
                        The fonctions is called with the following parameters:
            scans_0     Direction 0 scan counter
            scans_1     Direction 1 scan counter
            bad_scans   Bad scan counter
            acq_num     Sequene number

            Returns     = 0:    
                            NO_ERROR          Continue
                       != 0:
                                              Abort acquisition and return
                                              error code 
        Returns         = 0:
                            NO_ERROR          Everything is ok
                       != 0:
                            ERROR_SETUP       Error during setup
                            NOT_ENOUGH_MEMORY Not enough memory
                            ERROR_START       Error during start
                            ERROR_ACQ         Error during acquisition
                            TIMEOUT           Too much time between valid data
                            ...
Description
        Gets a coadded, apodized, complex spectrum and it's phase from the
        interferometer.
        dsp96_install must have been called prior to using this function.
        To do background acquisition, the "wait" parameter should be FALSE
        and dsp96_wait_end_coad() should be called to grab the collected data.
        Each buffer will contain 2 spectra representing the two separate
        directions. The npts field will contain the size of one spectrum even
        though the buffer will be twice that size.

Cautions
        Do not forget to release the spectrum buffers with bo_free() when
        it is no longer in use. If "wait" is set to FALSE, never attempt to
        manipulate data in "spec" before calling dsp96_wait_end_coadd()
        otherwise unpredictable results could occur.
#$%!i........................................................................*/

short dsp96_get_raw_spec(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r,
						YDATA *phs_i, word nbr_scans, long nbr_acq,
						double delay, float user_sna, float user_sxa,
                        float user_snb, float user_sxb, short apodization,
                        char wait, double *acq_time,
						short (*userwait)(word scans_0, word scans_1,
											word bad_scans, long acq_num))
{
	long scn_num_old = 0;
	long acq_num_old = 0;
	time_t end_time;
	long s1, s2;
	long double tmp;
	short ret, m;
	short year, month, day;
	long answer_len;
	Op_cond op_cond;

	if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return (ERROR_SETUP);
		}

	decode_status (status.status, &setup);
	setup.a.delay = det_delays[0];
	setup.b.delay = det_delays[1];
	encode_status (setup, cmd);

	if (seq32_tx_data (cmd, 6, LOAD_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return(ERROR_SETUP);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}
	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		dsp96_reset();
		return (ERROR);
		}

	scans = nbr_scans;
	cur_acq_num = 0L;

	if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
		{
		dsp96_reset();
		return (ERROR_SETUP);
		}
	decode_status (status.status, &setup);

	npts[0] = npts[1] = 16384L / setup.resolution;
	p_npts[0] = p_npts[1] = 128;

	tmp = (2 * p_npts[0]) / bo_flaser;
	s1  = (long)(user_sna * tmp);
	s2  = (long)(user_sxa * tmp + 0.99f);
	if (s1 >= p_npts[0]) s1 = 0;
	if (s2 >= p_npts[0]) s2 = p_npts[0]-1;
	p_firstx[0] = (double)(s1 / tmp);
	p_lastx[0]  = (double)(s2 / tmp);
	p_npts[0]   = s2 - s1 + 1;

	tmp = (2 * npts[0]) / bo_flaser;
	s1  = (long)(user_sna * tmp);
	s2  = (long)(user_sxa * tmp + 0.99f);
	if (s1 >= npts[0]) s1 = 0;
	if (s2 >= npts[0]) s2 = npts[0]-1;
	firstx[0] = (double)(s1 / tmp);
	lastx[0]  = (double)(s2 / tmp);
	npts[0]   = s2 - s1 + 1;

	/* must be here because s1 is overwritten */
	op_cond.spec_1st   = s1;

	tmp = (2 * p_npts[1]) / bo_flaser;
	s1  = (long)(user_snb * tmp);
	s2  = (long)(user_sxb * tmp + 0.99f);
	if (s1 >= p_npts[1]) s1 = 0;
	if (s2 >= p_npts[1]) s2 = p_npts[1]-1;
	p_firstx[1] = (double)(s1 / tmp);
	p_lastx[1]  = (double)(s2 / tmp);
	p_npts[1]   = s2 - s1 + 1;

	tmp = (2 * npts[1]) / bo_flaser;
	s1  = (long)(user_snb * tmp);
	s2  = (long)(user_sxb * tmp + 0.99f);
	if (s1 >= npts[1]) s1 = 0;
	if (s2 >= npts[1]) s2 = npts[1]-1;
	firstx[1] = (double)(s1 / tmp);
	lastx[1]  = (double)(s2 / tmp);
	npts[1]   = s2 - s1 + 1;

	size[0] = npts[0] * sizeof (float);
	size[1] = npts[1] * sizeof (float);
	p_size[0] = p_npts[0] * sizeof (float);
	p_size[1] = p_npts[1] * sizeof (float);

	op_cond.interf_scale = 2.5 / 32768.0f;

	op_cond.n_seq      = nbr_acq;
	op_cond.n_scans    = nbr_scans;
	op_cond.coad_delay = (long)(delay*10000);
	op_cond.spec_len   = npts[0];
	spec2_1st          = s1;
	spec2_len          = npts[1];
	op_cond.spec_apod  = apodization;
	op_cond.phase_len  = 128;
	op_cond.phase_apod = GAUSSIAN;
	op_cond.cflags     = CFLG_OP | CFLG_CHANNEL_A | CFLG_DIR_0;
	acq_type = 1;

	if (seq32_tx_data (&op_cond, 10, LOAD_X, OP_COND_OFF) != NO_ERROR)
		{
		dsp96_reset ();
		return (ERROR_START);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}

	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		dsp96_reset ();
		return (ERROR_START);
		}
	outport (io_addr, inport (io_addr) | HST_PC2);
	decode_status (status.status, &setup);

	num_det = (!!setup.a.present) + (!!setup.b.present);
	if (num_det == 0)
		{
		dsp96_reset ();
		return (ERROR_ACQ);
		}

	if (wait)
		{
		bo_timer_start (&end_time, TIMEOUT_DELAY);
		do
			{
			/* read status directly from memory */
			if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
				{
				goto no_stat;
				}
			if (userwait != NULL)
				{
				bo_acquire_96000_time =
					(status.delay-(status.irgb-status.irgb_start))/10000.0;
				if (status.scans_0 || status.scans_1)
					bo_acquire_96000_time = 0.0;

				ret = userwait ((word)status.scans_0, (word)status.scans_1,
								(word)status.scans_bad, status.seq_ctr);
				if (ret != NO_ERROR)
					{
					dsp96_hard_abort ();
					return (ret);
					}
				}
			bo_wait(0.1f);
			if (status.seq_ctr != acq_num_old ||
				status.scans_0+status.scans_1 != (long)scn_num_old)
				{
				bo_timer_start (&end_time, TIMEOUT_DELAY);
				acq_num_old = status.seq_ctr;
				scn_num_old = status.scans_0 + status.scans_1;
				}
no_stat:
			if (bo_timer_get (&end_time) == TRUE)
				{
				dsp96_reset ();
				return (TIMEOUT);
				}
			} while (!(inport (io_addr) & HST_M2) );

		bo_acquire_96000_time = 0.0;

		if (seq32_rx_data (&status, MB_STATUS_LEN, DUMP_X, 0) != NO_ERROR)
			{
			dsp96_reset ();
			return (ERROR_START);
			}
		decode_status (status.status, &setup);

		/* acquisition completed, send EOA flags to DSP */
		cmd[0] = CFLG_EOA | CFLG_CSPEC;
		if (seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF) != NO_ERROR)
			{
			dsp96_reset ();
			return (ERROR_ACQ);
			}
		// wait for response with a timeout
		bo_timer_start(&end_time, TIMEOUT_DELAY);
		while (!(inport (io_addr) & HST_M1) )
			{
			if (bo_timer_get (&end_time) == TRUE)
				{
				dsp96_reset ();
				return (TIMEOUT);
				}
			}

		/* get spectrum header */
		ret = seq32_get_data (&header, SCAN_HDR_LEN, &answer_len);
		if (ret != NO_ERROR || answer_len != SCAN_HDR_LEN)
			{
			dsp96_reset ();
			return (ERROR_ACQ);
			}

		/* time and other parameters at end of acquisition run */
		bo_getdate (&year, &month, &day);
		*acq_time = bo_get_time_t (year, month, day, 0,0,0) +
							header.irgb/10000.0;

		m = scans ? 2 : 1; /* buffers are half size in 0 scan mode */
		if ((phs_r->buffer = (float *)bo_alloc (m*p_size[0])) == NULL)
			{
			dsp96_reset ();
			return (NOT_ENOUGH_MEMORY);
			}
		if ((phs_i->buffer = (float *)bo_alloc (m*p_size[0])) == NULL)
			{
			free_ydata (*phs_r);
			dsp96_reset ();
			return (NOT_ENOUGH_MEMORY);
			}
		if ((spc_r->buffer = (float *)bo_alloc (m*size[0])) == NULL)
			{
			free_ydata (*phs_r);
			free_ydata (*phs_i);
			dsp96_reset ();
			return (NOT_ENOUGH_MEMORY);
			}
		if ((spc_i->buffer = (float *)bo_alloc (m*size[0])) == NULL)
			{
			free_ydata (*phs_r);
			free_ydata (*phs_i);
			free_ydata (*spc_r);
			dsp96_reset ();
			return (NOT_ENOUGH_MEMORY);
			}

		phs_r->firstx = phs_i->firstx = p_firstx[0];
		phs_r->lastx  = phs_i->lastx  = p_lastx[0];
		phs_r->npts   = phs_i->npts   = p_npts[0];
		spc_r->firstx = spc_i->firstx = firstx[0];
		spc_r->lastx  = spc_i->lastx  = lastx[0];
		spc_r->npts   = spc_i->npts   = npts[0];

		/* phase no longer available */

		// wait for response with a timeout
		bo_timer_start(&end_time, TIMEOUT_DELAY);
		while (!(inport (io_addr) & HST_M1) )
			{
			if (bo_timer_get (&end_time) == TRUE)
				{
				dsp96_reset ();
				return (TIMEOUT);
				}
			}
		ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
		if (ret != NO_ERROR || answer_len != spc_r->npts)
			{
			free_ydata (*phs_r);
			free_ydata (*phs_i);
			free_ydata (*spc_r);
			free_ydata (*spc_i);
			dsp96_reset ();
			return (ERROR_ACQ);
			}
		// wait for response with a timeout
		bo_timer_start(&end_time, TIMEOUT_DELAY);
		while (!(inport (io_addr) & HST_M1) )
			{
			if (bo_timer_get (&end_time) == TRUE)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				free_ydata (*spc_i);
				dsp96_reset ();
				return (TIMEOUT);
				}
			}
		ret	= seq32_get_data (spc_i->buffer, spc_i->npts, &answer_len);
		if (ret != NO_ERROR || answer_len != spc_i->npts)
			{
			free_ydata (*phs_r);
			free_ydata (*phs_i);
			free_ydata (*spc_r);
			free_ydata (*spc_i);
			dsp96_reset ();
			return (ERROR_ACQ);
			}

		if (scans)
			{
			/* phase no longer available */

			// wait for response with a timeout
			bo_timer_start(&end_time, TIMEOUT_DELAY);
			while (!(inport (io_addr) & HST_M1) )
				{
				if (bo_timer_get (&end_time) == TRUE)
					{
					free_ydata (*phs_r);
					free_ydata (*phs_i);
					free_ydata (*spc_r);
					free_ydata (*spc_i);
					dsp96_reset ();
					return (TIMEOUT);
					}
				}
			ret	= seq32_get_data (spc_r->buffer+spc_r->npts, spc_r->npts,
								  &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				free_ydata (*spc_i);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			// wait for response with a timeout
			bo_timer_start(&end_time, TIMEOUT_DELAY);
			while (!(inport (io_addr) & HST_M1) )
				{
				if (bo_timer_get (&end_time) == TRUE)
					{
					free_ydata (*phs_r);
					free_ydata (*phs_i);
					free_ydata (*spc_r);
					free_ydata (*spc_i);
					dsp96_reset ();
					return (TIMEOUT);
					}
				}
			ret	= seq32_get_data (spc_i->buffer+spc_i->npts, spc_i->npts,
								  &answer_len);
			if (ret != NO_ERROR || answer_len != spc_i->npts)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				free_ydata (*spc_i);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			}
		}
	return(NO_ERROR);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_COPY
File:   ACQ_96.C
Author: Thomas Buijs
Date:   June 1992

Synopsis
        #include "dsp96_pc.h"

        short dsp96_copy(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r,
                         YDATA *phs_i, short *scans_0, short *scans_1,
                         long *acq_num);

        spc_r           Copy of the coaddition buffer
                                Interferogram or
                                real spectrum
        spc_i           Copy of the imaginary spectrum
        phs_r           Copy of the real phase spectrum
        phs_i           Copy of the imaginary phase spectrum
        scans_0         Direction 0 scan counter associated with "coadbuf"
        scans_1         Direction 1 scan counter associated with "coadbuf"
        acq_num         Sequence number of current coad

        Return          NO_ERROR
                        NOT_ENOUGH_MEMORY if not enough space for copy

Description
        Take a copy of the current state of the coaddition buffers.
        The coaddition buffer will be an interferogram if dsp96_get_int()
        was called or a phase corrected spectrum if dsp96_get_spec() was used
        to start the acquisition. If dsp96_get_raw_spec() was used then the
        non phase corrected complex spectrum and the phase are returned.
        Do not forget to free the buffers with free_ydata() when they are no
        longer in use.
#$%!i........................................................................*/

short dsp96_copy(YDATA *spc_r, YDATA *spc_i, YDATA *phs_r, YDATA *phs_i,
								word *scans_0, word *scans_1, long *acq_num)
{
	short ret, m;
	long answer_len;
	float HPTR *tbuf;
	time_t end_time;

	cmd[0] = copy_channel;
	switch (acq_type)
		{
		case 2:	/* phase corrected spectrum */
			cmd[0] |= CFLG_PHASE;
		case 1:	/* complex spectrum */
			cmd[0] |= CFLG_CSPEC; 
		case 0: /* interferogram */
			cmd[0] |= CFLG_CPY;
		}
	if (seq32_tx_data (cmd, 1, LOAD_X, CFLG_OFF) != NO_ERROR)
		{
		dsp96_reset ();
		return (ERROR_ACQ);
		}
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}

	ret = seq32_get_data (&status, MB_STATUS_LEN, &answer_len);
	if (ret != NO_ERROR || answer_len != MB_STATUS_LEN)
		{
		dsp96_reset();
		return (ERROR);
		}
	decode_status (status.status, &setup);

	bo_acquire_96000_time = max(0.0,
		(status.delay-(status.irgb-status.irgb_start))/10000.0);
	if (status.scans_0 || status.scans_1) bo_acquire_96000_time = 0.0;

	*scans_0   = (word)status.scans_0;
	*scans_1   = (word)status.scans_1;
	*acq_num   = status.seq_ctr;
	// wait for response with a timeout
	bo_timer_start(&end_time, TIMEOUT_DELAY);
	while (!(inport (io_addr) & HST_M1) )
		{
		if (bo_timer_get (&end_time) == TRUE)
			{
			dsp96_reset ();
			return (TIMEOUT);
			}
		}

	m = scans ? 2 : 1; /* buffers are half size in 0 scan mode */
	switch(acq_type)
		{
		case 0: /* interferogram */
			if ((spc_r->buffer = (float *)bo_alloc (m*size[0])) == NULL)
				{
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}

			spc_r->npts = npts[0];
			spc_r->firstx = 0.0;
			spc_r->lastx = (bo_flaser/2.0);

			ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*spc_r);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			if (scans)
				{
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE)
						{
						free_ydata (*spc_r);
						dsp96_reset ();
						return (TIMEOUT);
						}
					}
				ret	= seq32_get_data (spc_r->buffer+spc_r->npts, spc_r->npts,
									  &answer_len);
				if (ret != NO_ERROR || answer_len != spc_r->npts)
					{
					free_ydata (*spc_r);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				}
			break;
		case 1: /* complex spectrum */
			if ((phs_r->buffer = (float *)bo_alloc (m*p_size[0])) == NULL)
				{
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			if ((phs_i->buffer = (float *)bo_alloc (m*p_size[0])) == NULL)
				{
				free_ydata (*phs_r);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			if ((spc_r->buffer = (float *)bo_alloc (m*size[0])) == NULL)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			if ((spc_i->buffer = (float *)bo_alloc (m*size[0])) == NULL)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}

			phs_r->firstx = phs_i->firstx = p_firstx[0];
			phs_r->lastx  = phs_i->lastx  = p_lastx[0];
			phs_r->npts   = phs_i->npts   = p_npts[0];
			spc_r->firstx = spc_i->firstx = firstx[0];
			spc_r->lastx  = spc_i->lastx  = lastx[0];
			spc_r->npts   = spc_i->npts   = npts[0];

			/* phase no longer available */

			ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				free_ydata (*spc_i);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
			// wait for response with a timeout
			bo_timer_start(&end_time, TIMEOUT_DELAY);
			while (!(inport (io_addr) & HST_M1) )
				{
				if (bo_timer_get (&end_time) == TRUE)
					{
					free_ydata (*phs_r);
					free_ydata (*phs_i);
					free_ydata (*spc_r);
					free_ydata (*spc_i);
					dsp96_reset ();
					return (TIMEOUT);
					}
				}
			ret	= seq32_get_data (spc_i->buffer, spc_i->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_i->npts)
				{
				free_ydata (*phs_r);
				free_ydata (*phs_i);
				free_ydata (*spc_r);
				free_ydata (*spc_i);
				dsp96_reset ();
				return (ERROR_ACQ);
				}

			if (scans)
				{
				/* phase no longer available */

				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE)
						{
						free_ydata (*phs_r);
						free_ydata (*phs_i);
						free_ydata (*spc_r);
						free_ydata (*spc_i);
						dsp96_reset ();
						return (TIMEOUT);
						}
					}
				ret	= seq32_get_data (spc_r->buffer+spc_r->npts, spc_r->npts,
									  &answer_len);
				if (ret != NO_ERROR || answer_len != spc_r->npts)
					{
					free_ydata (*phs_r);
					free_ydata (*phs_i);
					free_ydata (*spc_r);
					free_ydata (*spc_i);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE)
						{
						free_ydata (*phs_r);
						free_ydata (*phs_i);
						free_ydata (*spc_r);
						free_ydata (*spc_i);
						dsp96_reset ();
						return (TIMEOUT);
						}
					}
				ret	= seq32_get_data (spc_i->buffer+spc_i->npts, spc_i->npts,
									  &answer_len);
				if (ret != NO_ERROR || answer_len != spc_i->npts)
					{
					free_ydata (*phs_r);
					free_ydata (*phs_i);
					free_ydata (*spc_r);
					free_ydata (*spc_i);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				}
			break;
		case 2: /* spectrum */
			if ((spc_r->buffer = (float *)bo_alloc (size[0])) == NULL)
				{
				dsp96_reset ();
				return (NOT_ENOUGH_MEMORY);
				}
			spc_r->firstx = firstx[0];
			spc_r->lastx  = lastx[0];
			spc_r->npts   = npts[0];

			ret	= seq32_get_data (spc_r->buffer, spc_r->npts, &answer_len);
			if (ret != NO_ERROR || answer_len != spc_r->npts)
				{
				free_ydata (*spc_r);
				dsp96_reset ();
				return (ERROR_ACQ);
				}
 			if (scans)
				{
				if ((tbuf = (float *)bo_alloc (size[0])) == NULL)
					{
					free_ydata (*spc_r);
					dsp96_reset ();
					return (NOT_ENOUGH_MEMORY);
					}
				// wait for response with a timeout
				bo_timer_start(&end_time, TIMEOUT_DELAY);
				while (!(inport (io_addr) & HST_M1) )
					{
					if (bo_timer_get (&end_time) == TRUE)
						{
						free_ydata (*spc_r);
						dsp96_reset ();
						return (TIMEOUT);
						}
					}
				ret	= seq32_get_data (tbuf, spc_r->npts, &answer_len);
				if (ret != NO_ERROR || answer_len != spc_r->npts)
					{
					free_ydata (*spc_r);
					bo_free (tbuf);
					dsp96_reset ();
					return (ERROR_ACQ);
					}
				v_add (tbuf, spc_r->buffer, spc_r->buffer, spc_r->npts);
				if (*scans_0 && *scans_1)
					{
					v_scale (spc_r->buffer, 0.5f, spc_r->buffer, spc_r->npts);
					}
				bo_free (tbuf);
				}	
			break;
		}

	return(NO_ERROR);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1992

Name:   DSP96_SELECT_DETECTOR
File:   ACQ_96.C
Author: Francois Gallichand
Date:   January 4th, 1995

Synopsis
        #include "seq32_pc.h"

        short dsp96_select_detector (short detector);

        detector        The detector to be selected for the dsp96_copy()
                        function. Available values are:
                                0 for channel A
                                1 for channel B

        Return          NO_ERROR
                        ERROR_INV_DET if selected detector is not available

Description
		Select the channel to be used for dsp96_copy() function. Returns an
        error if the selected detector is not available.

Cautions
        The function dsp96_install() must have been called successfully prior
        to calling this function; otherwise may return invalid return code.
#$%!i........................................................................*/

short dsp96_select_detector (short detector)
{

	// Verify if the desired detector is present
	if (detector == 0 && !setup.a.present) return (ERROR_INV_DET);
	if (detector == 1 && !setup.b.present) return (ERROR_INV_DET);

	if (detector < 0 || detector > 1 ) return (ERROR_INV_DET);

	// Set the proper value according to the detector selected
	copy_channel = (detector==0) ? CFLG_CHANNEL_A : CFLG_CHANNEL_B;
	return (NO_ERROR);

}
