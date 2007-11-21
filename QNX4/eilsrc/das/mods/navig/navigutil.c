#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "nortlib.h"
#include "navigutil.h"

long ascii_navig( char *data, int correct_prec  ) {
  long val = 0;
  int count_prec = -1;
  int adjust_prec = 0;
  int i;
  int negative = 0;
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
  while ( ! isspace(*data) ) {
	if ( isdigit(*data) ) {
	  val = val*10 + *data - '0';
	  if ( count_prec >= 0 ) count_prec++;
	} 
	else if ( *data == '.' ) count_prec=0;
	else break;
	data++;
  }
  if ( count_prec == -1 ) count_prec = 0;
  adjust_prec = correct_prec - count_prec;
  if (adjust_prec > 0)
    for ( i=0; i<adjust_prec; i++) val *= 10;
  if (adjust_prec < 0)
    for ( i=0; i>adjust_prec; i--) val /= 10;
  if ( negative ) val = -val;
  return val;
}

#define MAX_WIDTH 12
char *navig_ascii( long int val, int width, int prec, char pos, char neg ) {
  static char ascbuf[MAX_WIDTH*2];
  int negative = 0, i, i0, j, w;
  
  assert( width > 0 && width <= MAX_WIDTH );
  assert( prec > 0 && prec < MAX_WIDTH-2);
  if ( val < 0 ) {
	negative = 1;
	val = -val;
  }
  i = i0 = MAX_WIDTH-1;
  j = -prec;
  while ( j <= 0 || val != 0 ) {
	ascbuf[i--] = val % 10 + '0';
	val = val/10;
	if ( ++j == 0 ) ascbuf[i--] = '.';
  }
  if ( negative && pos == ' ' ) ascbuf[i--] = neg;
  w = width - i0 + i;
  if ( w < 0 ) {
	strcpy( ascbuf, "+NAN" );
	for ( i = 4; i < width; i++ )
	  ascbuf[i] = ' ';
	ascbuf[width] = '\0';
	return ascbuf;
  }
  if ( pos != ' ' ) {
	while ( w-- > 0 ) ascbuf[i--] = ' ';
	ascbuf[i+1] = negative ? neg : pos;
  } else {
	while ( w-- > 0 ) ascbuf[++i0] = ' ';
  }
  ascbuf[++i0] = '\0';
  return ascbuf+i+1;
}
