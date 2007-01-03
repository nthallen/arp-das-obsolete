/* BOMIKEST.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 BOMIKEST.H 15-Jun-95,9:14:26,`TBUIJS'
     New object that decodes the status information of a standard Michelson
     spectrometer.
1:1 BOMIKEST.H 29-Jun-95,19:07:14,`TBUIJS'
     Added the base 2 log of the number of points in the status decoding.
1:2 BOMIKEST.H 9-Jan-96,13:48:40,`TBUIJS'
     Change the status mask to support ADVANCE FTIR and SUBMB type instruments
     that have extra status bits for detector saturation.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __BOMIKEST_H
#define __BOMIKEST_H


// This class is used to decode the hardware status word send bu the MB
// series michelson.

class BoMikestatus
	{
		union
			{
			unsigned short status;
			struct
				{
				unsigned short res   : 3; // resolution
				unsigned short       : 1;
				unsigned short sf    : 1; // sample per fringe
				unsigned short       : 1;
				unsigned short err   : 1; // error flag
				unsigned short dir   : 1; // direction flag
				unsigned short speed : 1; // speed flag
				unsigned short       : 4;
				unsigned short det   : 1; // detector flag
				} stat_bits;
			};
	public:
		short resolution;	// resolution in cm-1
		short res_code;		// resolution code from mike
		short samples;	    // samples per fringe 1 or 2
		short error;		// error flag 1=in error 0=no error
		short direction;	// direction indicator can be 0 or 1
		short speed;		// speed indicator 0=low speed 1=high speed
		short aux_det;	    // detector flag 0=internal 1=external
		unsigned long npts; // number of points in interferogram
		short log2;			// log to base 2 of the number of points

		BoMikestatus ();
		short new_status (unsigned short nstat);
	};


/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMikestatus::BoMikestatus
File:   BOMIKEST.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
		BoMikestatus::BoMikestatus ()

Description
		Constructor used to initialise the status values.
 #$%!........................................................................*/

inline BoMikestatus::BoMikestatus () : resolution (0), samples (0),
	error (0), direction (0), speed (0), aux_det (0), npts (0)
{
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   BoMikestatus::new_status
File:   BOMIKEST.H
Author: Thomas Buijs
Date:   June, 1995

Synopsis
		short BoMikestatus::new_status (unsigned short nstat)

        nstat           Raw status from mike

		returns         a flag indicating if the status is a velid mike
		                status. 0 = valid status, 1 = invalid status 

Description
		This function is used to decode and validate the raw Michelson
		status information. If the status is valid 0 is returned otherwise
		1 is returned. The status can be valid and even though an error
        is being signalled by the Michelson, so the error flag should be
		checked after using new_status.
		The new detector staturation flags used in the ADVANCE and SUBMB
		instruments are taken into account but not decoded
 #$%!........................................................................*/

inline short BoMikestatus::new_status (unsigned short nstat)
{
	status     = nstat;
	res_code   = stat_bits.res;
	resolution = 1 << (7 - res_code);
	samples    = (!stat_bits.sf) + 1;
	error      = !stat_bits.err;
	direction  = stat_bits.dir;
	speed      = stat_bits.speed;
	aux_det    = stat_bits.det;
	npts       = 65536L >> (8 - res_code - !stat_bits.sf);
	log2       = res_code + 7 + samples;
	return ((status & 0xd268) != 0xd260); // check bit key for valid pattern
}

#endif


