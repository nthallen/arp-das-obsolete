/* nl_dsp_dbl.h DSP definitions for doubles
 * $Log$
 * Revision 1.1  1994/12/03  14:22:47  nort
 * Initial revision
 *
 */
#ifndef _NL_DSP_DBL_H_INCLUDED
#define _NL_DSP_DBL_H_INCLUDED

#ifndef _NL_DSP_H_INCLUDED
  #include "nl_dsp.h"

  #define DD_VALUE_T double
  #define DD_STRUCT_T dd_struct_t(dbl)
  #define NEW_DIG_DELAY(x) new_dbl_dig_delay(x)
  #define DIG_DELAY(x,y) dbl_dig_delay(x,y)
  #define FREE_DIG_DELAY(x) free_dbl_dig_delay(x)
  #define DD_ID rcsid_digdly_dbl
#endif

decl_all_dig_dly(double, dbl)

typedef struct {
  dbl_dig_dly *dd;
  double sum;
} run_avg_t;
run_avg_t *new_run_average(unsigned short n_points);
double run_average(run_avg_t *ra, double v);
void free_run_average(run_avg_t *ra);

#endif
