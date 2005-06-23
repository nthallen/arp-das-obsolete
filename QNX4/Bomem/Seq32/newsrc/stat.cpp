/* STAT.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						    SOURCE CODE										*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 STAT.CPP 9-Dec-94,14:04:34,`CLAUDE' File creation
1:1 STAT.CPP 7-Feb-95,10:59:58,`CLAUDE' Fix decode_status dir flag bug
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif
/*
	Encode/decode MB200 status
	Version PEZ
*/
#include <string.h>
#include <math.h>
#include "useful.h"
#include "seq32_pc.h"

#include "stat.h"

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1994

Name:   MB_CMD
File:   STAT.CPP
Author: Claude Lafond
Date:   Dec 1, 1994

Synopsis
		#include "stat.h"

		void mb_cmd (const short instrument, const Mb200_setup &s,
					 long mb_cmd[6]);

		instrument		Instrument type (MR100, MR200)
		s				Requested setup
		mb_cmd			Resulting command string ready to be transmitted

Description
        Encode command MR100 and MR200 command string for FCOAD and ALIGN96
 #$%!........................................................................*/
void mb_cmd (const short instrument, const Mb200_setup &s, long mb_cmd[6])
{
static char tb_res[17]  = {0, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 4};
static char tb_gain[17] = {0, 3, 4, 0, 5, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 7};

short *status = (short *)mb_cmd;
char  *cmd    = (char *)&mb_cmd[3];

	memset (mb_cmd, 0, sizeof(mb_cmd)*6);	// Reset mb_cmd

	if (instrument == MR100)
		{
		status[0]  = 0xde60;
		status[0] |= 7 - (short)(log(s.resolution) / log(2.) + 0.1); // Resolution
		status[0] |= ((s.speed & 1) << 8);	// Speed setup

		if (!s.a.over)	   					// Oversampling setup
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
		}
	else
		{
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
			status[2]  =  tb_gain[s.b.gain];// Channel A gain setup
			status[2] |= (s.b.gain_1st << 4);
			cmd[5] 	   =  status[2];

			status[2] |= (s.b.id << 8);		// Detector A identification code setup
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
		}

	cmd[11] = 0x99;		 			// Key
}
/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1994

Name:   DECODE_STATUS
File:   STAT.CPP
Author: Claude Lafond
Date:   Dec 1, 1994

Synopsis
		#include "stat.h"

		void decode_status (const short instrument, const long *mb_status,
							Mb200_setup *s);

		instrument		Instrument type (MR100, MR200)
		mb_status		Status received from MR100 or MR200
		s				Resulting setup

Description
        Decode MR100 and MR200 status and convert it in a setup structure
 #$%!........................................................................*/
void decode_status (const short instrument, const long *mb_status,
					Mb200_setup *s)
{
static char tb_gain[8] = {1, 1, 1, 1, 2, 4, 8, 16};
short *status = (short *)mb_status;

	if (instrument == MR100)
		{
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
	else
		{
	//
	//	Decode first status word
	//
		s->resolution =  1 << (status[0] & 0x7);
		s->speed 	  = (status[0] >> 4) & 0xf;
		s->a.present  =  (status[0] & 0x100)  != 0;
		s->b.present  =  (status[0] & 0x200)  != 0;
		s->dir		  =  (status[0] & 0x400)  != 0;
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
}
