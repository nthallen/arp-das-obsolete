/* digdelay.c Parametrized Digital Delay filter
 * This provides a digital delay on an unsigned short datum (or
 * types compatible with that.) For other types, additional
 * functions will be required. Probably the ideal approach would
 * be making the bulk of this file an include file, digdelay.h
 * then making a number of short .c files which would define the
 * function names and the arg types. The structure, the three
 * functions and the type would need to be parametrized.
 */
#include "nortlib.h"
char DD_ID[] =
	"$Header$";

DD_STRUCT_T *NEW_DIG_DELAY(unsigned short n_points) {
  DD_STRUCT_T *dd;
  
  dd = new_memory(sizeof(DD_STRUCT_T));
  if (dd != 0) {
	dd->last_idx = dd->n_points = n_points;
	dd->value = new_memory(sizeof(DD_VALUE_T)*dd->n_points);
	if (dd->value == 0) {
	  free_memory(dd);
	  dd = NULL;
	}
  }
  return dd;
}

DD_VALUE_T DIG_DELAY(DD_STRUCT_T *dd, DD_VALUE_T v) {
  int i;
  DD_VALUE_T vout;
  
  if (dd->last_idx >= dd->n_points) {
	for (i = 0; i < dd->n_points; i++)
	  dd->value[i] = v;
	dd->last_idx = 0;
	vout = v;
  } else {
	if (++dd->last_idx >= dd->n_points)
	  dd->last_idx = 0;
	vout = dd->value[dd->last_idx];
	dd->value[dd->last_idx] = v;
  }
  return(vout);
}

void FREE_DIG_DELAY(DD_STRUCT_T *dd) {
  if (dd != 0) {
	if (dd->value != 0) free_memory(dd->value);
	free_memory(dd);
  }
}
