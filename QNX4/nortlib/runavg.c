/* runavg.c running average generic utility
 * $Log$
 */
#include "nl_dsp.h"
#include "nl_dsp_dbl.h"
#include "nortlib.h"

run_avg_t *new_run_average(unsigned short n_points) {
  run_avg_t *ra;
  
  ra = nl_new_memory(sizeof(run_avg_t));
  if (ra != 0) {
	ra->dd = NULL;
	ra->sum = n_points;
  }
  return ra;
}

double run_average(run_avg_t *ra, double v) {
  if (ra == 0) return 0;
  if (ra->dd == 0) {
	ra->dd = new_dbl_dig_delay((unsigned short) ra->sum);
	if (ra->dd != 0) {
	  dbl_dig_delay(ra->dd, v);
	  ra->sum *= v;
	  return v;
	} else return 0;
  } else {
	ra->sum -= dbl_dig_delay(ra->dd, v);
	ra->sum += v;
	return ra->sum/ra->dd->n_points;
  }
}

void free_run_average(run_avg_t *ra) {
  if (ra != 0) {
	if (ra->dd != 0)
	  free_memory(ra->dd);
	free_memory(ra);
  }
}
