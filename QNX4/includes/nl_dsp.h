/* medfilt.h Median filter module
 * $Log$
 * Revision 1.1  1994/11/22  14:54:14  nort
 * Initial revision
 *
 */
#ifndef _NL_DSP_H_INCLUDED
#define _NL_DSP_H_INCLUDED

typedef struct {
  unsigned short *value;
  unsigned short *rank;
  unsigned short *rank_idx;
  unsigned short n_points;
  unsigned short mid_index;
  unsigned short last_idx;
} med_filt;
med_filt *new_med_filter(unsigned short n_points);
unsigned short med_filter(med_filt *mf, unsigned short v);
void free_med_filter(med_filt *mf);

typedef struct {
  unsigned short *value;
  unsigned short n_points;
  unsigned short last_idx;
} dig_dly;
dig_dly *new_dig_delay(unsigned short n_points);
unsigned short dig_delay(dig_dly *dd, unsigned short v);
void free_dig_delay(dig_dly *dd);

#ifdef DD_VALUE_T
  typedef struct {
	DD_VALUE_T *value;
	unsigned short n_points;
	unsigned short last_idx;
  } DD_STRUCT_T;
  DD_STRUCT_T *NEW_DIG_DLY(unsigned short n_points);
  DD_VALUE_T DIG_DELAY(DD_STRUCT_T *dd, DD_VALUE_T v);
  void FREE_DIG_DELAY(DD_STRUCT_T *dd);
#endif

#define digdly_val(dd, n) (((n)>=dd->n_points)?0:\
  dd->value[dd->last_idx + ((n)>dd->last_idx?dd->n_points:0) - (n) ])
#define digdly_last(dd) dd->value[dd->last_idx]

#endif
