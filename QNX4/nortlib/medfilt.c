/* medfilt.c */
#include "nortlib.h"
#include "nl_dsp.h"
char rcsid_medfilt_c[] =
  "$Header$";

med_filt *new_gmed_filter(unsigned short n_points, int size) {
  med_filt *mf;
  
  if (n_points == 0) {
	if (nl_response)
	  nl_error(nl_response, "n_points must be non-zero in new_med_filter");
	return(NULL);
  }
  mf = new_memory(sizeof(med_filt));
  if (mf != 0) {
	mf->value = new_memory(size * n_points);
	mf->rank = new_memory(sizeof(unsigned short)*n_points);
	mf->rank_idx = new_memory(sizeof(unsigned short)*n_points);
	if (mf->value == 0 || mf->rank == 0 || mf->rank_idx == 0) {
	  free_med_filter(mf);
	  mf = NULL;
	}
	mf->rank[0] = mf->n_points = n_points;
	mf->mid_index = n_points/2;
	mf->last_idx = 0;
  }
  return mf;
}

void free_med_filter(med_filt *mf) {
  if (mf != 0) {
	if (mf->value != 0) free_memory(mf->value);
	if (mf->rank != 0) free_memory(mf->rank);
	if (mf->rank_idx != 0) free_memory(mf->rank_idx);
	free_memory(mf);
  }
}
