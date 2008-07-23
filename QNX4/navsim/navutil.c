#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "nortlib.h"
#include "navutil.h"

long ascii_nav( char *data ) {
  long val = 0;
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
	} else if ( *data != '.' ) {
	  nl_error( 1, "Invalid character in ascii_nav" );
	}
	data++;
  }
  if ( negative ) val = -val;
  return val;
}

#define MAX_WIDTH 12
char *nav_ascii( long int val, int width, int prec, char pos, char neg ) {
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
