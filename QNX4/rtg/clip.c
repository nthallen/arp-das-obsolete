/* clip.c contains clip_line();
 int clip_line(RtgGraph *graph, clip_pair *p1, clip_pair *p2);
 returns indication:
  0  specified line is totally within bounds
  1  only first point is out of bounds, clipped
  2  only second point is out of bounds, clipped
  3  both points are out of bounds, clipped
  4  entire line is totally out of bounds
  Also supplies new clipped coordinates

  clip_line is guaranteed that the incoming pairs are
  all numbers (no non-numbers)
*/
#include <assert.h>
#include "rtg.h"

static void clip_limit_chk(clip_coord *P, RtgAxis *ax) {
  P->flag = 0;
  if (P->clip < ax->min_limit)
	P->flag |= 1;
  else if (P->clip > ax->max_limit)
	P->flag |= 2;
}

int clip_line(RtgGraph *graph, clip_pair *P1, clip_pair *P2) {
  int swapped = 1, clipped = 0;
  clip_pair *Phold;
  RtgAxis *X_ax, *Y_ax;
  double dX, dY;

  for (;;) {
	if (!(P1->X.flag | P1->Y.flag | P2->X.flag | P2->Y.flag))
	  return clipped;
	if ((P1->X.flag & P2->X.flag) || (P1->Y.flag & P2->Y.flag))
	  return 4;
	if (!(P1->X.flag | P1->Y.flag)) {
	  Phold = P1; P1 = P2; P2 = Phold;
	  swapped = 2;
	}
	/* Now can assume P1 needs clipping */
	X_ax = graph->X_Axis;
	Y_ax = graph->Y_Axis;
	dX = P2->X.clip - P1->X.clip;
	dY = P2->Y.clip - P1->Y.clip;
	if (P1->Y.flag & 1) {
	  /* clip P1 to Ymin */
	  P1->X.clip += dX * ((Y_ax->min_limit - P1->Y.clip)/dY);
	  P1->Y.clip = Y_ax->min_limit;
	  clipped |= swapped;
	  P1->Y.flag &= ~1;
	  clip_limit_chk(&P1->X, X_ax);
	} else if (P1->Y.flag & 2) {
	  /* clip P1 to Ymax */
	  P1->X.clip += dX * ((Y_ax->max_limit - P1->Y.clip)/dY);
	  P1->Y.clip = Y_ax->max_limit;
	  clipped |= swapped;
	  P1->Y.flag = 0;
	  clip_limit_chk(&P1->X, X_ax);
	} else if (P1->X.flag & 1) {
	  /* clip P1 to Xmin */
	  P1->Y.clip += dY * ((X_ax->min_limit - P1->X.clip)/dX);
	  P1->X.clip = X_ax->min_limit;
	  clipped |= swapped;
	  P1->X.flag &= ~1;
	  clip_limit_chk(&P1->Y, Y_ax);
	} else {
	  assert(P1->X.flag & 2);
	  /* clip P1 to Xmax */
	  P1->Y.clip += dY * ((X_ax->max_limit - P1->X.clip)/dX);
	  P1->X.clip = X_ax->max_limit;
	  clipped |= swapped;
	  P1->X.flag = 0;
	}
  }
}
