/* VECTOR.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/* This software is the property of Bomem and should be considered and      */
/* treated as proprietary information.  Refer to the "Source Code License   */
/* Agreement"                                                               */
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 VECTOR.H 2-Apr-93,15:27:02,`THOMAS' File creation
1:1 VECTOR.H 15-Apr-94,9:29:24,`JEAN' Added the new TLIB header
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif


#ifndef	BOMEM_VECTOR
#define	BOMEM_VECTOR

#ifndef	BOMEM_USEFUL
	#include "USEFUL.H"
#endif


#define	V_NO_COMMON_RANGE		(ERROR_BASE_VECTOR-1)
/* Invalid convolution parameter */
#define	V_CNV_INVALID_PARM		(ERROR_BASE_VECTOR-2)


/* Define round-up strategy where applicable */
#define BO_ROUNDUP		1		
#define BO_ROUNDDN		-1
#define BO_ROUND		0

/* FUNCTION PROTOTYPES */

										/* VMINMAX.C */
void v_min_max  (float HPTR *vin, float *min, float *max, long length);
										/* VLMINMAX.C */
void v_lmin_max (long  HPTR *vin, long  *min, long  *max, long length);
										/* VSMINMAX.C */
void v_smin_max (short HPTR *vin, short *min, short *max, long length);
										/* VIMINMAX.C */
void v_imin_max (int HPTR *vin, int *min, int *max, long length);
										/* VMINMAXP.C */
void v_min_maxp (float HPTR *vin, float *min, float *max,
				 long *minp, long *maxp, long length);

										/* V_F_TO_I.C */
void v_f_to_i (float HPTR *data_fl, int HPTR *data_int, long npts, 
			   float sc_fact);
										/* V_I_TO_F.C */
void v_i_to_f (int HPTR *data_int, float HPTR *data_fl, long npts,
					  float sc_factor);
										/* V_F_TO_L.C */
void v_f_to_l (float HPTR *data_fl, long HPTR *data_int, long npts,
					  float sc_fact);
										/* V_L_TO_F.C */
void v_l_to_f (long HPTR *data_int, float HPTR *data_fl, long npts,
					  float sc_fact);
										/* V_L_TO_I.C */
void v_l_to_i (long HPTR *data_l, int HPTR *data_int, long npts,
					  float sc_fact);
										/* FFT.C */
void fft (char inverse, float HPTR *x, float HPTR *y, long n);
										/* RUNSCR.C */
void runscr (float HPTR *x, float HPTR *y, long length);
										/* IRUNSCR.C */
void irunscr (float HPTR *x, float HPTR *y, long length);

										/* VRAW2ABS.C, VRAW2ABS.ASM */
#ifdef __cplusplus
extern "C" {
#endif

void v_raw_to_abs (float HPTR *ref, float HPTR *raw, float HPTR *abs,
				   long length, double base, float clip_pos, float clip_neg);
#ifdef __cplusplus
	}
#endif

#ifdef __WATCOMC__
	#pragma aux (ASM_RTN) v_raw_to_abs;
#endif

										/* VRAW2TRA.C */
void v_raw_to_tra(float HPTR *ref, float HPTR *raw, float HPTR *tra,
				  long length, float clip_pos, float clip_neg);
										/* V_ADD.C */
void v_add (float HPTR *vin1, float HPTR *vin2, float HPTR *vout, long length);
										/* V_SUB.C */
void v_sub (float HPTR *vin1, float HPTR *vin2, float HPTR *vout, long length);
										/* V_MUL.C */
void v_mul (float HPTR *vin1, float HPTR *vin2, float HPTR *vout, long length);
										/* V_DIV.C */
void v_div (float HPTR *vin1, float HPTR *vin2, float HPTR *vout, long length);
										/* V_DOT.C */
float v_dot_prod (float HPTR *vin1, float HPTR *vin2, long length);
										/* V_NEG.C */
void v_neg (float HPTR *vin, float HPTR *vout, long length);
										/* V_ABS.C */
void v_abs (float HPTR *vin, float HPTR *vout, long length);
										/* V_EXP.C */
void v_exp (float HPTR *vin, float HPTR *vout, long length);
										/* V_EXP_10.C */
void v_exp_10 (float HPTR *vin, float HPTR *vout, long length);
										/* V_INTEGR.C */
float v_integrate (float HPTR *vin, double delta, long length);
										/* V_2INTEG.C */
float v_2integrate (float HPTR *vin, double delta, long length);
										/* V_LN.C */
void v_ln (float HPTR *vin, float HPTR *vout, long length);
										/* V_LOG.C */
void v_log (float base_log, float HPTR *vin, float HPTR *vout, long length);
										/* V_LOG_10.C */
void v_log_10 (float HPTR *vin, float HPTR *vout, long length);
										/* V_POWER.C */
void v_power (float base, float HPTR *vin, float HPTR *vout, long length);
										/* V_SOLSUB.C */
void v_solsub (float HPTR *vin1, float HPTR *vin2, float factor,
               float HPTR *vout, long length);
										/* V_OFFSET.C */
void v_offset (float HPTR *vin, float factor, float HPTR *vout, long length);
										/* V_SCALE.C */
void v_scale (float HPTR *vin, float factor, float HPTR *vout, long length);
										/* V_SUM.C */
float v_sum (float HPTR *vin1, long length);
										/* V_AVG.C */
float v_avg (float HPTR *vin1, long length);
										/* V_STD.C */
float v_std (float HPTR *vin1, float average, long length);
										/* V_SUM_SQ.C */
float v_sum_sqr_dif (float HPTR *vin1, float HPTR *vin2, long length);
										/* VS_SUM_S.C */
float vs_sum_sqr_dif (float HPTR *vin1, float scalar, long length);
										/* V_SUMABS.C */
void v_sumabssub(float HPTR *vin1, float HPTR *vin2, float *sum, long length);
										/* V_FLIP.C */
void v_flip (float HPTR *vin, long npts);
										/* V_FQ2IDX.C */
long freq_to_index0 (double firstx, double lastx, long npts, double freq,
					 short rounding);
#define freq_to_index(fst, lst, npts, freq) freq_to_index0(fst, lst, npts, freq, BO_ROUNDUP)
										/* V_IDX2FQ.C */
double index_to_freq (double firstx, double lastx, long npts, long index);
										/* V_COMMON.C */
short get_common_range(YDATA spec1, YDATA spec2, YDATA *common1,
					   YDATA *common2);
										/* V_CLIP.C */
void v_clip (float HPTR *vin, float HPTR *vout, float min,
			 float max, long length);



										/* V_NORM.C */
short normalize_spc(YDATA *data, float sc_fact, float reg1, float reg2,
					char absolute);

										/* V_MANIP.c */
#define BO_MANIP_ADD		0
#define BO_MANIP_COAD		1			/* Use for higher level function */
#define BO_MANIP_ALOG		2
#define BO_MANIP_CLIP		3
#define BO_MANIP_EXP		4
#define BO_MANIP_KUBEL		5
#define BO_MANIP_LN			6
#define BO_MANIP_LOG		7
#define BO_MANIP_MAG		8
#define BO_MANIP_MULT		9
#define BO_MANIP_OFFSET		10
#define BO_MANIP_PHASE		11
#define BO_MANIP_RATIO		12
#define BO_MANIP_SCALE		13
#define BO_MANIP_SUB		14
#define BO_MANIP_SPEC		15
#define BO_MANIP_TEMP		16
#define BO_MANIP_CM2UM		17
#define BO_MANIP_SUBR		18

short v_manip (short fnc_code, float HPTR *vout,
			   long npts, float cn, float cx, float HPTR *ref);


/* ----------------------------- */
/* Convolution related functions */			   
/* ----------------------------- */
#define CONVOLVE_ID    1		/* File type identification */
#define CONVOLVE_VALID 100		/* Coefficient version number * 100 */

typedef	struct
	{
	char id[80];		/* ASCII text describing the filter parameters */
	char ctrlz;			/* Contain CTRL-Z so that we can type the id */
	short fileid;		/* File identification number */
	short version;		/* Version number * 100 */
	short n_coef;		/* # of coefficient to use for convolution */
	float *coef;		/* Coefficient vector */
	}Cnv_coef;

/* Function in V_CNV.C */
short convolve(char *coeff_file, YDATA *abs);
short v_cnv_read(char *filename, Cnv_coef *coef);
short v_cnv (YDATA *vin, Cnv_coef *coef, YDATA *vout);
void v_cnv_end(Cnv_coef *coef);

/* Function in V_CNVCOE.C */
short v_cnv_coef(short d_order, short p_order, short npts, Cnv_coef *cnv);
short v_cnv_save(char *filename, Cnv_coef coef);
			   
/*  NON OPERATIONAL FUNCTIONS
void v_derivative (float HPTR *vin, double delta, float HPTR *vout,
                   long length);
void v_kubelka (float HPTR *vin, float HPTR *vout, long length,
                float kub_cutoff);
*/

/* stucture for xy paired data */
/* Data type associated with XY data */
typedef struct
 	{
	double HPTR *xbuffer;    /* pointer to x values */
	float  HPTR *ybuffer;	 /* pointer to y values (one alloc for x & y) */
	long npts;				 /* number of xy points in buffers */
	} Xydata;

/* functions in v_xydata.c */
short alloc_xydata(long npts, Xydata *xydata);
void free_xydata(Xydata xydata);
short v_y_to_xy(YDATA in, Xydata *out);
short v_xy_to_y(Xydata in, YDATA *out);


#endif
