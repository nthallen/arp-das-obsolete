/* ratio.c averages the absolute value of the given spectra over two
   regions and calculates the ratio between the two averages.
   The result is either displayed or printed.
   References spec_collected, plot_dir, windows globals
*/
#include <assert.h>
#include <windows/Qwindows.h>
#include <math.h>
#include "seq32_pc.h"
#include "bomem.h"
#include "bomemw.h"

#define A_MIN 2000.
#define A_MAX 3000.
#define B_MIN 750.
#define B_MAX 1250.

static long interp(double f, YDATA *yd) {
  long int i;

  i = yd->npts * (f - yd->firstx)/(yd->lastx - yd->firstx);
  if (i < 0) i = 0;
  else if (i >= yd->npts) i = yd->npts - 1;
  return i;
}

static double average_region(double minf, double maxf,
		  YDATA *spec_r, YDATA *spec_i) {
  long int i, imin, imax;
  double sum, pr, pi;
  float HPTR *yr, *yi;

  /* Determine indexes for min and max */
  imin = interp(minf, spec_r);
  imax = interp(maxf, spec_r);

  /* Calculate average */
  yr = spec_r->buffer;
  yi = spec_i->buffer;
  if (plot_dir) {
	yr += spec_r->npts;
	yi += spec_i->npts;
  }
  sum = 0.;
  for (i = imin; i <= imax; i++) {
	pr = yr[i];
	pi = yi[i];
	sum += sqrt(pr*pr + pi*pi);
  }
  return sum/(imax-imin+1);
}

void ratio_regions(YDATA *spec_r, YDATA *spec_i) {
  double avg_a, avg_b, ratio;

  if (spec_collected) {
	assert(spec_r->npts == spec_i->npts);
	avg_a = average_region(A_MIN, A_MAX, spec_r, spec_i);
	avg_b = average_region(B_MIN, B_MAX, spec_r, spec_i);
	ratio = avg_a/avg_b;
	if (windows) {
	  WindowCurrent(base_wind_id);
	  WindowBarCurrent('T', NULL);
	  ChangeOptions("r", "-d");
	  ChangeReal("r", ratio);
	  Draw();
	} else fprintf(stderr, "Ratio is %10.6lf\n", ratio);
  } else if (windows) {
	/* dim output */
	WindowCurrent(base_wind_id);
	WindowBarCurrent('T', NULL);
	ChangeOptions("r", "d");
	Draw();
  }
}
