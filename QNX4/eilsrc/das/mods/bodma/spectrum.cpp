/* SPECTRUM.CPP */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/*						        SOURCE CODE									*/
/* This software is the property of Bomem and should be considered and		*/
/* treated as proprietary information.  Refer to the "Source Code License 	*/
/* Agreement"																*/
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 SPECTRUM.CPP 26-Jun-95,9:59:30,`TBUIJS'
     Modified version of spectrum that supports double interferograms and uses
     BoMemory objects instead of YDATA structures. Only the new phase correction
     is implemented.
1:1 SPECTRUM.CPP 4-Jul-95,14:47:36,`TBUIJS'
     Added a function to take a complex spectrum and phase correct it into a
     spectrum.
1:2 SPECTRUM.CPP 12-Jul-95,11:43:50,`TBUIJS'
     Fixed a problem with array scalling in near IR mode. The original scalling
     was done in the complex spectrum routine, this worked fine for interferograms
     but not for converting raw spectra as returned by the DSP board. The
     scalling is now done in the phase correction instead.
1:3 SPECTRUM.CPP 11-Dec-95,16:09:56,`TBUIJS'
     Scale spectra up by a factor of 4 in the phase correction in order to match
     the amplitudes of spectra obtained with the current BOMEM librairy.
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <dos.h>

#include "spectrum.h"

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   SPECTRUM
File:   SPECTRUM.CPP
Author: Thomas Buijs, (original: Claude Lafond)
Date:   June 1995, (original: May 1, 1990)

Synopsis
        #include "spectrum.h"

        void spectrum (BoMemory<float> &interf, short dbl, double sf,
		               double *sn, double *sx, short apod_type,
					   short phase_apod, long phase_npts)

        interf          one or two double sided interferograms
		dbl             dbl flag to indicate whether there are 1 or 2
		                    interferograms.
							 dbl = 0 -> 1 interferogram
							 dbl = 2 -> 2 interferograms, first is zero
                 			 dbl = 1 -> 2 interferograms, second is zero
							 dbl = 3 -> 2 interferograms, both good
		sf              sampling frequency of interferogram encoded as
		                     sampling frequency for single interferogram
							 2*sampling frequency for double interferogram
		sn,sx			Spectral range requested, will return actual range
        apod_type       Apodization type
                                BOXCAR
                                BARTLET
                                COSINE
                                HAMMING
                                BLACKMAN
                                GAUSSIAN
                                NORTON_B_WEAK
                                NORTON_B_MEDIUM
                                NORTON_B_STRONG
        phase_apod		Apodization type for phase
                                BOXCAR
                                BARTLET
                                COSINE
                                HAMMING
                                BLACKMAN
                                GAUSSIAN
                                NORTON_B_WEAK
                                NORTON_B_MEDIUM
                                NORTON_B_STRONG
        phase_npts      Number of points to be apodized in interf.

Description
        This function computes the spectrum of an interferogram. The spectrum
		is moved to the beginning of the interferogram buffer and the
		size is adjusted.

Cautions
        This function assumes that:
                ZPD is at interf.npts / 2
                interf.npts is a power of 2
                phase_npts is a power of 2
                phase_npts <= interf.npts

        The resulting spectrum data overwrites the original interferogram
        buffer.  

See also
        complex_spec
 #$%!........................................................................*/

void spectrum (BoMemory<float> &interf, short dbl, double sf, double *sn,
				double *sx, short apod_type, short phase_apod,
				long phase_npts)
{
	BoMemory<float> phase, spec1;
	long spec_r, spec_i, spec_n;
	float  *specr,  *speci,  *phr,  *phi,  *specf;
	double t, tsn, tsx;
	long i, npts;

	// separate the two interferograms, if not a double interferogram
	// only use interf
	switch (dbl)
		{
		case 1:
			// only first contains data
			sf *= 0.5;
			npts = interf.size () / 2;
			interf.resize (npts);
			break;
		case 2:
			// only second contains data
			sf *= 0.5;
			npts = interf.size () / 2;
			interf.copy_block (interf, 0, npts, npts);
			interf.resize (npts);
			break;
		case 3:
			// both contain data
			sf *= 0.5;
			npts = interf.size () / 2;
			spec1.resize (npts);
			spec1.copy_block (interf, 0, npts, npts);
			interf.resize (npts);
			break;
		default:
			break;
		}

	// build phase interferogram for direction 0
	phase = interf;

	// apodize and zero fill direction 0
	apodize (phase, phase_npts / 2, phase_apod);

	// compute phase FFT direction 0
	tsn = *sn;
	tsx = *sx;
	complex_spec (phase, sf, &tsn, &tsx, &spec_r, &spec_i, &spec_n);

	// Compute complex spectrum direction 0
	apodize (interf, interf.size () / 2, apod_type);
	tsn = *sn;
	tsx = *sx;
	complex_spec (interf, sf, &tsn, &tsx, &spec_r, &spec_i, &spec_n);

	// Phase correct and scale the spectrum. For compatibility with
	// previous generation Bomem drivers the final raw spectrum must
	// be scalled up by a factor of 4.
	specr = interf.p+spec_r;
	speci = interf.p+spec_i;
	phr = phase.p+spec_r;
	phi = phase.p+spec_i;
	if (sf > 8000)
		{
		// for oversampling scale down by a factor of 2
		for (i = spec_n + 1; --i; phr++, phi++, specr++, speci++)
			{
			t = 0.5 * sqrt (*phr * *phr + *phi * *phi);
			if (t>1e-7)
				{
				*specr = (*phr * *specr + *phi * *speci)/t;
				}
			}
		}
	else
		{
		for (i = spec_n + 1; --i; phr++, phi++, specr++, speci++)
			{
			t = 0.25 * sqrt (*phr * *phr + *phi * *phi);
			if (t>1e-7)
				{
				*specr = (*phr * *specr + *phi * *speci)/t;
				}
			}
		}

	/* Move spectrum to the beginning of the interferogram buffer */
	interf.copy_block (interf, 0, spec_r, spec_n);
	interf.resize (spec_n);

	// transform direction 1 if it exists
	if (dbl == 3)
		{
		// build phase interferogram for direction 0
		phase = spec1;

		// apodize and zero fill direction 0
		apodize (phase, phase_npts / 2, phase_apod);

		// compute phase FFT direction 0
		tsn = *sn;
		tsx = *sx;
		complex_spec (phase, sf, &tsn, &tsx, &spec_r, &spec_i, &spec_n);

		// Compute complex spectrum direction 0
		apodize (spec1, spec1.size () / 2, apod_type);
		tsn = *sn;
		tsx = *sx;
		complex_spec (spec1, sf, &tsn, &tsx, &spec_r, &spec_i, &spec_n);

		// Phase correct, scale and combine the spectrum. For compatibility
		// with previous generation Bomem drivers the final raw spectrum
		// must be scalled up by a factor of 4.
		specr = spec1.p+spec_r;
		speci = spec1.p+spec_i;
		phr = phase.p+spec_r;
		phi = phase.p+spec_i;
		specf = interf.p;
		if (sf > 8000)
			{
			// for oversampling scale down by a factor of 2
			for (i = spec_n + 1; --i; phr++, phi++, specr++, speci++, specf++)
				{
				t = 0.5 * sqrt (*phr * *phr + *phi * *phi);
				if (t>1e-7)
					{
					*specf = 0.5*(*specf + (*phr * *specr + *phi * *speci)/t);
					}
				else
					{
					*specf = 0.5 * (*specf + *specr);
					}
				}
			}
		else
			{
			for (i = spec_n + 1; --i; phr++, phi++, specr++, speci++, specf++)
				{
				t = 0.25 * sqrt (*phr * *phr + *phi * *phi);
				if (t>1e-7)
					{
					*specf = 0.5*(*specf + (*phr * *specr + *phi * *speci)/t);
					}
				else
					{
					*specf = 0.5 * (*specf + *specr);
					}
				}
			}
		}
	*sn = tsn;
	*sx = tsx;
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   SPECTRUM
File:   SPECTRUM.CPP
Author: Thomas Buijs
Date:   June 1995

Synopsis
        #include "spectrum.h"

        void spectrum (BoMemory<float> &spec, long interf_npts, long npts,
		               short dbl, double sf, double *sn, double *sx,
					   short phase_apod, long phase_npts)

        spec            one or two complex spectra with interferogram
						   center burts: real, imaginary, phase
		interf_npts     number of points in original interferogram
		npts            number of points in the spectrum
		dbl             dbl flag to indicate whether there are 1 or 2
		                    spectra.
							 dbl = 0 -> 1 spectrum
							 dbl = 2 -> 2 spectra, first is zero
                 			 dbl = 1 -> 2 spectra, second is zero
							 dbl = 3 -> 2 spectra, both good
		sf              sampling frequency of interferogram encoded as
		                     sampling frequency for single interferogram
							 2*sampling frequency for double interferogram
		sn,sx			Spectral range requested, will return actual range
        phase_apod		Apodization type for phase
                                BOXCAR
                                BARTLET
                                COSINE
                                HAMMING
                                BLACKMAN
                                GAUSSIAN
                                NORTON_B_WEAK
                                NORTON_B_MEDIUM
                                NORTON_B_STRONG
        phase_npts      Number of points to be apodized in interf.

Description
        This function computes the real phase corrected spectrum given a
		complex spectrum and the zpd region of the original interferogram.
		The resulting spectrum will be at the beginning of the buffer and
		the buffer size will be adjusted to hold npts points.

Cautions
        This function assumes that:
                ZPD is at interf.npts / 2
                interf.npts is a power of 2
                phase_npts is a power of 2
                phase_npts <= interf.npts

        The resulting spectrum data overwrites the original data in the
        buffer.  

See also
        complex_spec
 #$%!........................................................................*/

void spectrum (BoMemory<float> &spec, long interf_npts, long npts, short dbl,
				double sf, double *sn, double *sx, short phase_apod,
				long phase_npts)
{
	BoMemory<float> phase, spec1;
	long spec_r, spec_i, spec_n;
	float  *specr,  *speci,  *phr,  *phi,  *specf;
	double t, tsn, tsx;
	long i;

	// separate the two directions, if not a double spectrum
	// only use spec
	switch (dbl)
		{
		case 1:
			// only first contains data
			sf *= 0.5;
			spec.resize (npts*2+phase_npts);
			break;
		case 2:
			// only second contains data
			sf *= 0.5;
			spec.copy_block (spec, 0, npts*2+phase_npts, npts*2+phase_npts);
			spec.resize (npts*2+phase_npts);
			break;
		case 3:
			// both contain data
			sf *= 0.5;
			spec1.resize (npts*2+phase_npts);
			spec1.copy_block (spec, 0, npts*2+phase_npts, npts*2+phase_npts);
			spec.resize (npts*2+phase_npts);
			break;
		default:
			break;
		}

	// build phase interferogram for direction 0
	phase.resize (interf_npts);
	phase.copy_block (spec, interf_npts/2 - phase_npts/2, npts*2, phase_npts);

	// apodize and zero fill direction 0
	apodize (phase, phase_npts / 2, phase_apod);

	// compute phase FFT direction 0
	tsn = *sn;
	tsx = *sx;
	complex_spec (phase, sf, &tsn, &tsx, &spec_r, &spec_i, &spec_n);

	// Phase correct and scale the spectrum. For compatibility with
	// previous generation Bomem drivers the final raw spectrum must
	// be scalled up by a factor of 4.
	specr = spec.p;
	speci = spec.p+npts;
	phr = phase.p+spec_r;
	phi = phase.p+spec_i;
	if (sf > 8000)
		{
		// for oversampling scale down by a factor of 2
		for (i = spec_n + 1; --i; phr++, phi++, specr++, speci++)
			{
			t = 0.5 * sqrt (*phr * *phr + *phi * *phi);
			if (t>1e-7)
				{
				*specr = (*phr * *specr + *phi * *speci)/t;
				}
			}
		}
	else
		{
		for (i = spec_n + 1; --i; phr++, phi++, specr++, speci++)
			{
			t = 0.25 * sqrt (*phr * *phr + *phi * *phi);
			if (t>1e-7)
				{
				*specr = (*phr * *specr + *phi * *speci)/t;
				}
			}
		}

	// Resize buffer to hold only real spectrum
	spec.resize (spec_n);

	// transform direction 1 if it exists
	if (dbl == 3)
		{
		// build phase interferogram for direction 0
		phase.copy_block (spec1, interf_npts/2 - phase_npts/2, npts*2,
																phase_npts);

		// apodize and zero fill direction 0
		apodize (phase, phase_npts / 2, phase_apod);

		// compute phase FFT direction 0
		tsn = *sn;
		tsx = *sx;
		complex_spec (phase, sf, &tsn, &tsx, &spec_r, &spec_i, &spec_n);

		// Phase correct, scale and combine the spectrum. For compatibility
		// with previous generation Bomem drivers the final raw spectrum
		// must be scalled up by a factor of 4.
		specr = spec1.p;
		speci = spec1.p+npts;
		phr = phase.p+spec_r;
		phi = phase.p+spec_i;
		specf = spec.p;
		if (sf > 8000)
			{
			// for oversampling scale down by a factor of 2
			for (i = spec_n + 1; --i; phr++, phi++, specr++, speci++, specf++)
				{
				t = 0.5 * sqrt (*phr * *phr + *phi * *phi);
				if (t>1e-7)
					{
					*specf = 0.5*(*specf + (*phr * *specr + *phi * *speci)/t);
					}
				else
					{
					*specf = 0.5 * (*specf + *specr);
					}
				}
			}
		else
			{
			for (i = spec_n + 1; --i; phr++, phi++, specr++, speci++, specf++)
				{
				t = 0.25 * sqrt (*phr * *phr + *phi * *phi);
				if (t>1e-7)
					{
					*specf = 0.5*(*specf + (*phr * *specr + *phi * *speci)/t);
					}
				else
					{
					*specf = 0.5 * (*specf + *specr);
					}
				}
			}
		}
	*sn = tsn;
	*sx = tsx;
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   APODIZE
File:   APODIZE.C
Author: Claude Lafond
Date:   May 1, 1990

Synopsis
        #include "spectrum.h"

        void apodize (BoMemory<float> &interf, long apod_npts,
															short apod_type)

        interf          Buffer for interferogram
        apod_npts       Number of points to be apodized
                        Normally:  interf.npts / 2
        apod_type       Apodization function type
                                BOXCAR
                                BARTLET
                                COSINE
                                HAMMING
                                BLACKMAN
                                GAUSSIAN
                                NORTON_B_WEAK
                                NORTON_B_MEDIUM
                                NORTON_B_STRONG

Description
        This function apodizes the input interferogram around ZPD for the
        number of points given by apod_npts. Note that the number of points
        (apod_npts) does not have to be a power of 2.
        If apod_npts < interf.npts, zero filling is done at both ends of
        the interferogram even when BOXCAR apodization is used.

Cautions
        apod_npts must be <= interf.npts / 2

Examples
        This code will produce an interferogram of 16384 with a ZPD at 8192
            interf[0..8091] = 0
            interf[8292..16383] = 0
        Only 200 points in the center of the interferogram are apodized, the
        others are set to zero thus reducing the resolution.
        
        YDATA interf;

            interf_syn (&interf, 16384, 1);
            apodize (interf, 100, HAMMING);

See also
        spectrum

Internal informations
		The center of the apodization function is set at npts/2 +1 

 #$%!i........................................................................*/

void apodize (BoMemory<float> &interf, long apod_npts, short apod_type)
{
	float  *pr,  *pl;
	float a, factor, step, step2, step_pi;
	long i, len2, apod_npts_p1;

	// zero fill both ends
	len2 = interf.size () >> 1;
	interf.init_block (0.0f, 0, len2-apod_npts);
	interf.init_block (0.0f, len2+apod_npts, len2-apod_npts);


	pl = pr = interf.p + len2;				/* ZPD */
	step	= 1.0f / apod_npts;
	step2	= step * step;
	step_pi	= (float)(step * M_PI);
	apod_npts_p1 = apod_npts+1;

	factor = 1.0f; 							/* Center value is 1.0 */

	switch (apod_type)
		{
		case BOXCAR:
			return;							/* Do nothing */

		case BARTLET:
			for (i = 1; i < apod_npts_p1; i++)
				{
				*pr++ *= factor;
				*pl-- *= factor;
				factor = 1.0f - i*step;
				}
			break;

		case COSINE:
			for (i = 1; i < apod_npts_p1; i++)
				{
				*pr++ *= factor;
				*pl-- *= factor;
				factor = (float)(0.5f + 0.5f*cos(i*step_pi));
				}
			break;

		case HAMMING:
			for (i = 1; i < apod_npts_p1; i++)
				{
				*pr++ *= factor;
				*pl-- *= factor;
				factor = (float)(0.53856f + 0.46144f*cos(i*step_pi));
				}
			break;

		case BLACKMAN:
			step2 = 2.f * step_pi;
			for (i = 1; i < apod_npts_p1; i++)
				{
				*pr++ *= factor;
				*pl-- *= factor;
				factor = (float)(0.42323f + 0.49755f*cos(i*step_pi) +
								            0.07922f*cos(i*step2));
				}
			break;

		case GAUSSIAN:
			step *= (-3.91202300543f*step);
			for (i = 1; i < apod_npts_p1; i++)
				{
				*pr++ *= factor;
				*pl-- *= factor;
				factor = (float)exp (i*(float)i*step);
				}
			break;

		case NORTON_B_WEAK:
			for (i = 1; i < apod_npts_p1; i++)
				{
				*pr++ *= factor;
				*pl-- *= factor;
				a = 1.f - i*(float)i*step2;
				factor = 0.548f - 0.0833f*a + 0.5353f*a*a;
				}
			break;

		case NORTON_B_MEDIUM:
			for (i = 1; i < apod_npts_p1; i++)
				{
				*pr++ *= factor;
				*pl-- *= factor;
				a = 1.f - i*(float)i*step2;
				factor = 0.261f - 0.154838f*a + 0.894838f*a*a;
				}
			break;

		case NORTON_B_STRONG:
			for (i = 1; i < apod_npts_p1; i++)
				{
				*pr++ *= factor;
				*pl-- *= factor;
				a = 1.f - i*(float)i*step2;
				a *= a;
				factor = 0.09f + 0.5875f*a + 0.3225f*a*a;
				}
			break;
		}

	// Process first interferogram point
	*pl *= factor;					  

	// Process last point if it is there
	if (apod_npts * 2 < interf.size ())
		{
		*pr *= factor;	  
		}
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   COMPLEX_SPEC
File:   SPECTRUM.C
Author: Claude Lafond
Date:   May 1, 1990

Synopsis
        #include "spectrum.h"

        void complex_spec (BoMemory<float> &interf, double sf, double *sn,
		                   double *sx, long *spec_r, long *spec_i,
						   long *spec_npts)

        interf          Double sided interferogram
		sf              Sampling frequency

        sn              User requested spec_r and spec_i sigma minimum in cm-1
        sx              User requested spec_r and spec_i sigma maximum in cm-1

        spec_r          Index to real part of the resulting spectrum
        spec_i          Index to imaginary part of the resulting spectrum
        spec_npts       Number of points in spec_r and spec_i

Description
        This function computes the complex spectrum from an interferogram.
        The resulting spectrum is left in interf, so the original interferogram
        data is overwritten!!!

Cautions
        This function assumes that:
                ZPD is at interf.npts / 2
                interf.npts is a power of 2
                apod_npts <= interf.npts

See also
        spectrum
 #$%!i........................................................................*/

void complex_spec (BoMemory<float> &interf, double sf, double *sn, double *sx,
				   long *spec_r, long *spec_i, long *spec_npts)
{
	long len2;
	long i;
	float  *p;

	*spec_npts = interf.size ();
	len2 = *spec_npts >> 1;
	
	// Compute real FFT
	scramble (interf, *spec_npts);

	p = interf.p+len2;
	for (i = len2 + 1; --i; p++)
		{
		*p = -*p;
		}
	frxfm (short (log(len2)/log(2.0)+0.5), interf.p, interf.p+len2);
	p = interf.p+len2;
	for (i = len2 + 1; --i; p++)
		{
		*p = -*p;
		}
	runscr (interf.p, interf.p+len2, len2);

	comp_limits (sn, sx, spec_npts, sf, spec_r, spec_i);
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   SCRAMBLE
File:   SCRAMBLE.C
Author: Claude Lafond
Date:   May 1, 1990

Synopsis
        #include "spectrum.h"

        void scramble (BoMemory<float> &interf, long npts);

        interf          Interferogram to be scrambled
        npts            Number of points in interf

Description
        Function used to "scramble" an interferogram into the two
        input buffers used by the fft function.  The normal use of these
        two buffers is to contain the real and imaginary parts of the
        input signal, but since an interferogram is always a real signal,
        this artifact can be used to limit the amount of memory needed to
        perform the calculation.

        The interferogram is defined in angle values ranging from -pi
        to +pi rad., whereas the fft is defined over the 0 to +2pi range.
        The input interferogram thus has to be rearranged in order to
        have the zpd point as the first point in the buffer.  Scrambling
        takes that rearranged interferogram and places its even data points
        in the real buffer, and its odd data points in the imaginary buffer.

        If C(n) is the interferogram defined from 0 to 2 pi
                and  i= 0 .. 2 pi
                            ------
                              N
        then:
            r_buf= C(2n)
            i_buf= C(2n+1)

See also
        spectrum
 #$%!i........................................................................*/

void scramble (BoMemory<float> &interf, long npts)
{
	long i;			 		/* Loop indexes */

	long n4 = npts / 4;

	BoMemory<float> tmp (npts);
	float  *pr = interf.p + npts/2;

	for (i=0; i <  n4;  i++) tmp.p[i] = interf.p[1 + (i<<1)];
	for (i=1; i <= n4;  i++) pr[-i] = pr[-(i<<1)];
	for (i=0; i <  n4;  i++) interf.p[i] = pr[i<<1];
	for (i=0; i <  n4;  i++) pr[i] = pr[1 + (i<<1)];
	for (i=0; i <  n4;  i++) pr[n4+i] = tmp.p[i];
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   IRUNSCR
File:   IRUNSCR.C
Author: Claude Lafond
Date:   May 1, 1990

Synopsis
        #include "vector.h"

        void irunscr (float  *x, float  *y, long npts);

        x               Real component
        y               Imaginary component
        npts            Number of points in x and y (must be a power of 2)

Description
        Function to unscramble the output data of the inverse fft.  It must
        be used to recover actual data when the inverse FFT input is a real 
        scrambled vector.

See also
        fft, runscr

Examples
        If you have a vector v of N real numbers you can compute its complex
        inverse FFT with:
                for (i=0; i < N; i+=2) x[i/2] = v[i];
                for (i=0; i < N; i+=2) y[i/2] = v[i+1];
                fft (TRUE, x, y, N/2);
                irunscr (x, y, N/2);
 #$%!i........................................................................*/

void irunscr (float  *x, float  *y, long npts)
{
double step, tstep;
float  *xl,  *xr,  *yl,  *yr;
float xx, bx, yy, by, c, s, t;
long i;

	*x = 0.5f * (*x + *y);				/* Unscramble first point */
	*y = 0.0f;

	xl = &x[1];
	xr = &x[npts-1];
	yl = &y[1];
	yr = &y[npts-1];

	step = M_PI / npts;

	for (i=1; i <= npts/2; i++)
		{
		tstep = i*step;
		c = (float)cos (tstep);
		s = (float)sin (tstep);

		xx = *xl - *xr;
		bx = *xl + *xr;
		yy = *yl + *yr;
		by = *yl - *yr;

		t = yy*c + xx*s;
		*xl++ = .25f * (bx + t);
		*xr-- = .25f * (bx - t);

		t = xx*c - yy*s;
		*yl++ = .25f * ( by - t);
		*yr-- = .25f * (-by - t);
		}
}

/*#$%!i*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   RUNSCR
File:   RUNSCR.C
Author: Claude Lafond
Date:   May 1, 1990

Synopsis
        #include "vector.h"

        void runscr (float  *x, float  *y, long npts);

        x               Real component
        y               Imaginary component
        npts            Number of points in x and y (must be a power of 2)

Description
        Function to unscramble the output data of the fft.  This function must
        be used to recover actual data when the FFT input is a real scrambled 
        vector.

See also
        fft, irunscr

Examples
        If you have a vector v of N real numbers you can compute its complex FFT
        with:
                for (i=0; i < N; i+=2) x[i/2] = v[i];
                for (i=0; i < N; i+=2) y[i/2] = v[i+1];
                fft (FALSE, x, y, N/2);
                runscr (x, y, N/2);
 #$%!i........................................................................*/

void runscr (float  *x, float  *y, long npts)
{
	double step, tstep;
	float  *xl,  *xr,  *yl,  *yr;
	float xx, bx, yy, by, c, s, t;
	long i;

	*x = *x + *y;
	*y = 0.0f;

	xl = &x[1];
	xr = &x[npts-1];
	yl = &y[1];
	yr = &y[npts-1];

	step = M_PI / npts;

	for (i=1; i <= npts/2; i++)
		{
		tstep = i*step;
		c = (float)cos (tstep);
		s = (float)sin (tstep);

		xx = *xl - *xr;
		bx = *xl + *xr;
		yy = *yl + *yr;
		by = *yl - *yr;

		t = yy*c - xx*s;
		*xl++ = .5f * (bx + t);
		*xr-- = .5f * (bx - t);

		t = xx*c + yy*s;
		*yl++ = .5f * ( by - t);
		*yr-- = .5f * (-by - t);
		}
}

/*#$%!*************************************************************************
                     COPYRIGHT (C) BOMEM INC, 1990

Name:   COMP_LIMITS
File:   SPECTRUM.CPP
Author: Thomas Buijs
Date:   June, 1995

Synopsis
		#include "spectrum.h"

		void comp_limits (double *sn, double *sx, long *npts, double sf,
					long *spec_r, long *spec_i)

		sn              Desired sigma minimum, will be adjusted to nearest pt
		sx              Desired sigma maximum, will be adjusted to nearest pt
		npts            number of points in interferogram->in spectrum
		sf              sampling frequency
		spec_r			index of real spectrum
		spec_i			index of imaginary spectrum

Description
		Function to compute the limits of a spectrum given the sampling
		frequency and the number of points in the interferogram

See also
        fft, irunscr

 #$%!........................................................................*/

void comp_limits (double *sn, double *sx, long *npts, double sf,
					long *spec_r, long *spec_i)
{
	long s1, s2, len2;

	// index and number of points
	len2 = *npts / 2;
	s1 = (long) ceil (len2 * *sn / sf - 0.999f);
	s2 = (long) floor (len2 * *sx / sf + 0.999f);
	if (s1 >= len2) s1 = len2 - 1;
	if (s2 >= len2) s2 = len2 - 1;
	*npts = s2 - s1 + 1;

	*spec_r = s1;
	*spec_i = len2 + s1;

	// Spectral range
	*sn = s1 * sf / len2;
	*sx = s2 * sf / len2;
}

