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

/* These are hard coded */
typedef struct {
  unsigned short *value;
  unsigned short n_points;
  unsigned short last_idx;
} dig_dly;
dig_dly *new_dig_delay(unsigned short n_points);
unsigned short dig_delay(dig_dly *dd, unsigned short v);
void free_dig_delay(dig_dly *dd);

/* here are the generic macros */
#define dd_struct_t(abbr) abbr##_dig_dly
#define def_dd_struct(type,abbr) typedef struct { type *value;\
  unsigned short n_points;\
  unsigned short last_idx;\
} dd_struct_t(abbr)
#define decl_new_dig_dly(type,abbr) \
  dd_struct_t(abbr) *new_##abbr##_dig_delay(unsigned short n_points)
#define decl_dig_dly(type,abbr) \
  type abbr##_dig_delay(dd_struct_t(abbr) *dd, type v)
#define decl_free_dig_dly(abbr) \
  void free_##abbr##_dig_delay(dd_struct_t(abbr) *dd)
#define decl_all_dig_dly(type,abbr) \
  def_dd_struct(type,abbr);\
  decl_new_dig_dly(type,abbr);\
  decl_dig_dly(type,abbr);\
  decl_free_dig_dly(abbr);

#define digdly_val(dd, n) (((n)>=dd->n_points)?0:\
  dd->value[dd->last_idx + ((n)>dd->last_idx?dd->n_points:0) - (n) ])
#define digdly_last(dd) dd->value[dd->last_idx]

#endif
