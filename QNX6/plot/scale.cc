#include "plot.h"

scale::scale() {
  pixels = 0;
  min = max = 0;
}

void scale::set() {
  if ( max != min ) {
	scalev = pixels/(max-min);
  } else scalev = 0;
}

void scale::set( int pixels ) {
  this->pixels = pixels;
  set();
}

void scale::set( float minv, float maxv ) {
  min = minv;
  max = maxv;
  set();
}

void scale::set( f_matrix *data ) {
  float minv = 0, maxv = 0;
  int r, c, nr = data->nrows, nc = data->ncols;
  float *col;

  if ( nr > 0 && nc > 0 ) {
    minv = maxv = data->vdata[0];
	for ( c = 0; c < nc; c++ ) {
	  col = data->mdata[c];
	  for ( r = 0; r < nr; r++ ) {
	    if ( col[r] < minv ) minv = col[r];
	    else if ( col[r] > maxv ) maxv = col[r];
	  }
	}
  }
  set( minv, maxv );
}

int scale::evaluate(float val) {
  int rv = int((val-min)*scalev);
  if ( direction == 0 ) rv = pixels - rv;
  return rv;
}
