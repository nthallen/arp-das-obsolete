/* SPECTRUM.H */

#ifndef	BOMEM_SPECTRUM
#define	BOMEM_SPECTRUM

#ifndef	BOMEM_USEFUL
	#include "USEFUL.H"
#endif

/*
	Type of apodization (new values should always be positive T.Buijs 1992)
*/
#define BOXCAR			0
#define BARTLET			1
#define COSINE			2
#define HAMMING			3
#define BLACKMAN		4
#define GAUSSIAN		5
#define NORTON_B_WEAK	6
#define NORTON_B_MEDIUM	7
#define NORTON_B_STRONG	8

										/* SPECTRUM.C */
short spectrum (YDATA interf,

				short apod_type, long apod_npts,
				double user_firstx, double user_lastx,
				char stored_phase, short phase_apod_type,

				float *phase_r, float *phase_i,
				long phase_npts, double phase_firstx, double phase_lastx,

				YDATA *spec);

void complex_spec (YDATA interf,

				   double user_firstx, double user_lastx,

				   float HPTR **spec_r, float HPTR **spec_i,
				   long *spec_npts, double *spec_firstx, double *spec_lastx);

										/* SCRAMBLE.C */
void scramble (float HPTR *interf, long npts);

										/* APODIZE.C */
void apodize (YDATA interf, long apod_npts, short apod_type);

										/* PHASECOR.ASM */
#ifdef __cplusplus
extern "C" {
#endif

void phase_cor (float HPTR *spec_r, float HPTR *spec_i, double spec_sn,
				double spec_sx, double spec_delta,
				float *phase_r, float *phase_i, double phase_sn,
				double phase_sx, double phase_delta,
				float scale, float HPTR *cspec);

#ifdef __cplusplus
	}
#endif

										/* INTERFSY.C */
short interf_syn (YDATA *interf, long npts, float max_value);

#ifdef __WATCOMC__
	#pragma aux (ASM_RTN) phase_cor;
#endif

#endif
