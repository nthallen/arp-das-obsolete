/* medfiltg.c
   Generic median filter function that can be extended to
   different types.
*/
#include "nortlib.h"
#include "nl_dsp.h"

#ifdef NEW_MED_FILTER
  med_filt *NEW_MED_FILTER( int n_points ) {
	return new_gmed_filter( n_points, sizeof(MF_VAL_T) );
  }
#endif

#ifndef MF_VAL_INTEGRAL
  #define MF_VAL_INTEGRAL 1
#endif

MF_VAL_T MED_FILTER(med_filt *mf, MF_VAL_T v) {
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
	#if MF_VAL_INTEGRAL
	  i = (iv%2) && (jv%2);
	  iv = (iv/2) + (jv/2);
	  if (i) iv++;
	  return iv;
	#else
	  return (iv+jv)/2;
	#endif
  }
}
