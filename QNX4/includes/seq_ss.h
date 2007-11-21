/* seq_ss.h
 * $Log$
 * Revision 1.1  1994/11/22  14:54:34  nort
 * Initial revision
 *
 * Revision 1.1  1993/12/02  20:52:47  nort
 * Initial revision
 *
 */
#ifndef _SEQ_SS_H_INCLUDED
#define _SEQ_SS_H_INCLUDED

#ifndef _SSP_H
  #error Must include ssp.h before seq_ss.h
#endif
void store_seq_ss(double v0, int col, double val);
sps_ptr open_seq_ss(char *prefix, int n_columns);
int close_seq_ss(int save_it);

#if defined __386__
#  pragma library (nortlib3r)
#elif defined __SMALL__
#  pragma library (nortlibs)
#elif defined __COMPACT__
#  pragma library (nortlibc)
#elif defined __MEDIUM__
#  pragma library (nortlibm)
#elif defined __LARGE__
#  pragma library (nortlibl)
#endif

#endif
