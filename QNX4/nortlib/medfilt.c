/* medfilt.c
 * $Log$
 * Revision 1.1  1994/06/14  15:27:01  nort
 * Initial revision
 *
 */
#include "nortlib.h"
#include "nl_dsp.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

#ifndef MF_VAL_T
  #define MF_VAL_T unsigned short
  #define MF_VAL_INTEGRAL 1
#endif

med_filt *new_med_filter(unsigned short n_points) {
  med_filt *mf;
  
  if (n_points == 0) {
	if (nl_response)
	  nl_error(nl_response, "n_points must be non-zero in new_med_filter");
	return(NULL);
  }
  mf = new_memory(sizeof(med_filt));
  if (mf != 0) {
	mf->value = new_memory(sizeof(MF_VAL_T)*n_points);
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

MF_VAL_T med_filter(med_filt *mf, MF_VAL_T v) {
  int i, j, ir, jr;
  MF_VAL_T iv, jv;

  /* If this is the first point, do the initialization */
  if (mf->rank[0] >= mf->n_points) {
	for (i = mf->n_points-1; i >= 0; i--) {
	  mf->value[i] = v;
	  mf->rank[i] = mf->rank_idx[i] = i;
	}
  }
  i = mf->last_idx;
  jv = mf->value[i];
  iv = mf->value[i] = v;
  ir = mf->rank[i];
  if (v > jv) {
	/* bubble sort up */
	for (jr = ir+1; jr < mf->n_points; ir++, jr++) {
	  j = mf->rank_idx[jr];
	  jv = mf->value[j];
	  if (iv > jv) {
		/* swap ranks for iv and jv */
		mf->rank[i] = jr;
		mf->rank[j] = ir;
		mf->rank_idx[ir] = j;
		mf->rank_idx[jr] = i;
	  } else break;
	}
  } else {
	/* bubble sort down */
	for (jr = ir-1; jr >= 0; ir--, jr--) {
	  j = mf->rank_idx[jr];
	  jv = mf->value[j];
	  if (iv < jv) {
		/* swap ranks for iv and jv */
		mf->rank[i] = jr;
		mf->rank[j] = ir;
		mf->rank_idx[ir] = j;
		mf->rank_idx[jr] = i;
	  } else break;
	}
  }
  
  /* increment the last index for the next point that gets added */
  if (++(mf->last_idx) == mf->n_points) mf->last_idx = 0;
  
  /* Now calculate the return value */
  if (mf->n_points & 1)
	return mf->value[mf->rank_idx[mf->mid_index]];
  else {
	iv = mf->value[mf->rank_idx[mf->mid_index]];
	jv = mf->value[mf->rank_idx[mf->mid_index-1]];
	#ifdef MF_VAL_INTEGRAL
	  i = (iv%2) && (jv%2);
	  iv = (iv/2) + (jv/2);
	  if (i) iv++;
	  return iv;
	#else
	  return (iv+jv)/2;
	#endif
  }
}

void free_med_filter(med_filt *mf) {
  if (mf != 0) {
	if (mf->value != 0) free_memory(mf->value);
	if (mf->rank != 0) free_memory(mf->rank);
	if (mf->rank_idx != 0) free_memory(mf->rank_idx);
	free_memory(mf);
  }
}
