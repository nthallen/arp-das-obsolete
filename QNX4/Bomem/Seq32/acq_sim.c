#include <windows/Qwindows.h>
#include <math.h>
#include <malloc.h>
#include "bomem.h"
#include "nortlib.h"

#define N_PTS_ACQ 32000L
struct {
  float huge *buffer;
  long int npts;
} dbuf;

void acquire_data(void) {
  long int i;

  if (dbuf.buffer == NULL) {
	dbuf.buffer = halloc(N_PTS_ACQ, sizeof(float));
	dbuf.npts = N_PTS_ACQ;
  }
  for (i = 0; i < dbuf.npts; i++) {
	dbuf.buffer[i] = sin((i*n_scans*3.14159)/N_PTS_ACQ);
  }
  plot_opt();
}

void plot_opt(void) {
  int i = plot_option * 10;
  if (dbuf.npts > i)
	new_plot(dbuf.buffer + i, dbuf.npts-i);
}
