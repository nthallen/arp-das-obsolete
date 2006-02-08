#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "nortlib.h"
#include "navutil.h"

long ascii_nav( char *data, int prec ) {
  long val = 0;
  int negative = 0;
  int decimals = -1;
  while ( isspace( *data) ) data++;
  if ( *data == 'N' || *data == 'E' ) {
	data++;
	while ( isspace(*data) ) data++;
  } else if ( *data == 'S' || *data == 'W' ) {
	negative = 1;
	data++;
	while ( isspace(*data) ) data++;
  } else if ( *data == '-' ) {
	negative = 1;
	data++;
  } else if (*data == '+') data++;
  if (strnicmp( data, "NAN", 3) == 0 ) return LONG_MAX;
  while ( ! isspace(*data) && decimals < prec ) {
	if ( isdigit(*data) ) {
	  val = val*10 + *data - '0';
	  if ( decimals >= 0 ) decimals++;
	} else if ( *data == '.' ) {
	  decimals = 0;
	} else break;
	data++;
  }
  if ( decimals < 0 ) decimals = 0;
  for ( ; decimals < prec; decimals++ ) val *= 10;
  if ( negative ) val = -val;
  return val;
}
