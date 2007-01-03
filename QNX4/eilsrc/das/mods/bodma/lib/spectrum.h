/* SPECTRUM.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 SPECTRUM.H 26-Jun-95,9:56:28,`TBUIJS'
     Modified version of spectrum that use BoMemory blocks instead of YDATA
     structures and only implements the new phase correction.
1:1 SPECTRUM.H 3-Jul-95,11:22:16,`TBUIJS'
     Updated version of spectrum contains prototypes for the new spectrum
     function that takes a raw spectrum and phase corrects it.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#ifndef __SPECTRUM_H
#define __SPECTRUM_H

#include "bomemory.h"

/*
	Type of apodization (new values should always be positive T.Buijs 1992)
*/
enum
	{
	BOXCAR			= 0,
	BARTLET			= 1,
	COSINE			= 2,
	HAMMING			= 3,
	BLACKMAN		= 4,
	GAUSSIAN		= 5,
	NORTON_B_WEAK	= 6,
	NORTON_B_MEDIUM	= 7,
	NORTON_B_STRONG	= 8
	};

void spectrum (BoMemory<float> &interf, short dbl, double sf, double *sn,
				double *sx, short apod_type, short phase_apod,
				long phase_npts);

void spectrum (BoMemory<float> &spec, long interf_npts, long npts, short dbl,
				double sf, double *sn, double *sx, short phase_apod,
				long phase_npts);

void apodize (BoMemory<float> &interf, long apod_npts, short apod_type);

void complex_spec (BoMemory<float> &interf, double sf, double *sn, double *sx,
				   long *spec_r, long *spec_i, long *spec_npts);

void scramble (BoMemory<float> &interf, long npts);

void irunscr (float *x, float  *y, long npts);
void runscr (float  *x, float  *y, long npts);

void comp_limits (double *sn, double *sx, long *npts, double sf,
					long *spec_r, long *spec_i);

void frxfm (unsigned short n2pow, float  *x, float  *y);


#endif
