/* nl_dsp_ul.h unsigned long DSP definitions
 * $Log$
 * Revision 1.1  1994/11/22  14:44:32  nort
 * Initial revision
 *
 */
#ifndef _NL_DSP_UL_H_INCLUDED
#define _NL_DSP_UL_H_INCLUDED

#ifndef _NL_DSP_H_INCLUDED
  #include "nl_dsp.h"

  #define DD_VALUE_T unsigned long
  #define DD_STRUCT_T dd_struct_t(ul)
  #define NEW_DIG_DELAY(x) new_ul_dig_delay(x)
  #define DIG_DELAY(x,y) ul_dig_delay(x,y)
  #define FREE_DIG_DELAY(x) free_ul_dig_delay(x)
#endif

decl_all_dig_dly(unsigned long, ul)

#endif
