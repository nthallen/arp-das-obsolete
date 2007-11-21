#include "tmctime.h"

short int tdrift( void ) {
  long int mytime, piftime;

  mytime = itime();
  piftime = P_data.pif_hh * 3600L + P_data.pif_mm * 60 + P_data.pif_ss;
  mytime %= (12 * 3600L);
  piftime = ( piftime + (18 * 3600L) - mytime ) % (12 * 3600L);
  return (short int) (piftime - (6 * 3600));
}
