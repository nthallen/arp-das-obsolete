/* elttype.c defines the various property element types
 */
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <windows/Qwindows.h>
#include "rtg.h"
#include "nortlib.h"

/*----------------------------------------------------------------
  String and Key functions
----------------------------------------------------------------*/
static void string_assign( RtgPropValue *to, RtgPropValue *from) {
  dastring_update(&to->text, from->text);
}

static void string_2pict( const char *tag, RtgPropValue *nv ) {
  ChangeText( tag, nv->text, 0, -1, 0);
}

static void elt_2string( RtgPropValue *nv ) {
  dastring_update(&nv->text, trim_spaces(ElementText()));
}

/* return non-zero if different */
static int string_compare( RtgPropValue *old, RtgPropValue *new ) {
  return strcmp(dastring_value(old->text), dastring_value(new->text));
}

/* Type-sensitive sanity check. Returns non-zero on failure
   Only called if the values have changed
 */
static int key_check( RtgPropDefB *PDB,
				  RtgPropValue *old, RtgPropValue *new) {
  if (ChanTree_Rename(PDB->def->tree, old->text, new->text) == 0) {
	nl_error(2, "Name %s already exists: cannot rename", new->text);
	return 1;
  }
  return 0;
}

static void key_recover( RtgPropDefB *PDB,
				RtgPropValue *old, RtgPropValue *new) {
  ChanTree_Rename(PDB->def->tree, new->text, old->text);
}

static void ascii_2string( RtgPropValue *val, const char *str) {
  dastring_update(&val->text, str);
}

static void string_2ascii(char *buf, RtgPropValue *val) {
  strcpy( buf, dastring_value( val->text ) );
}

/*----------------------------------------------------------------
  Long Functions
----------------------------------------------------------------*/
#ifdef NEED_LONG_TYPE
  static void long_assign( RtgPropValue *to, RtgPropValue *from) {
	to->long_int = from->long_int;
  }

  static int long_compare( RtgPropValue *old, RtgPropValue *new ) {
	return (old->long_int != new->long_int);
  }

  static void ascii_2long( RtgPropValue *val, const char *str) {
	val->long_int = atol(str);
  }
#endif

/*----------------------------------------------------------------
  Boolean Functions
----------------------------------------------------------------*/
static void bool_assign( RtgPropValue *to, RtgPropValue *from) {
  to->boolean = from->boolean;
}

static void bool_2pict( const char *tag, RtgPropValue *nv ) {
  ChangeState(tag, nv->boolean);
}

static void elt_2bool( RtgPropValue *nv ) {
  nv->boolean = ElementState();
}

static int bool_compare( RtgPropValue *old, RtgPropValue *new ) {
  return (old->boolean != new->boolean);
}

static void ascii_2bool( RtgPropValue *val, const char *str) {
  val->boolean = atoi(str) != 0;
}

static void bool_2ascii(char *buf, RtgPropValue *val) {
  strcpy( buf, val->boolean != 0 ? "1" : "0" );
}

/*----------------------------------------------------------------
  Exclusive Functions
----------------------------------------------------------------*/
static void excl_2pict( const char *tag, RtgPropValue *nv ) {
  char ltag[40];
  
  sprintf(ltag, "%s%d", tag, nv->boolean);
  ChangeState(ltag, 1);
}

static void elt_2excl( RtgPropValue *nv ) {
  if (ElementState() == 1) {
	char *tag;
	
	tag = ElementTag();
	assert(tag != 0);
	while (*tag != 0 && !isdigit(*tag)) tag++;
	nv->boolean = atoi(tag);
  }
}

/*----------------------------------------------------------------
  number/unsigned short functions
----------------------------------------------------------------*/
static void numus_assign( RtgPropValue *to, RtgPropValue *from) {
  to->ushort_int = from->ushort_int;
}

static void numus_2pict( const char *tag, RtgPropValue *nv ) {
  ChangeNumber( tag, nv->ushort_int);
}

static void elt_2numus( RtgPropValue *nv ) {
  nv->ushort_int = ElementNumber();
}

/* return non-zero if different */
static int numus_compare( RtgPropValue *old, RtgPropValue *new ) {
  return ( old->ushort_int != new->ushort_int );
}

static void ascii_2numus( RtgPropValue *val, const char *str) {
  val->ushort_int = atol(str);
}

/* utoa() is a WATCOM function to convert an unsigned int to string
   using a specified radix.
   utoa( unsigned int value, char *buffer, int radix);
*/
static void numus_2ascii(char *buf, RtgPropValue *val) {
  utoa( val->ushort_int, buf, 10 );
}

/*----------------------------------------------------------------
  number/real functions
----------------------------------------------------------------*/
static void real_assign( RtgPropValue *to, RtgPropValue *from) {
  to->real = from->real;
}

static void numreal_2pict( const char *tag, RtgPropValue *nv ) {
  ChangeReal( tag, nv->real);
}

static void textreal_2pict( const char *tag, RtgPropValue *nv ) {
  char buf[80];
  
  gcvt( nv->real, 8, buf );
  ChangeText( tag, buf, 0, -1, 0);
}

static void elt_2numreal( RtgPropValue *nv ) {
  nv->real = ElementReal();
}

static void elt_2textreal( RtgPropValue *nv ) {
  nv->real = atof( ElementText() );
}

/* return non-zero if different */
static int real_compare( RtgPropValue *old, RtgPropValue *new ) {
  return ( old->real != new->real );
}

static void ascii_2real( RtgPropValue *val, const char *str) {
  val->real = atof(str);
}

/* gcvt( double value, int ndigits, char *buffer ) is a WATCOM
   function to convert a double to a string similar to the %g
   printf format
*/
static void real_2ascii(char *buf, RtgPropValue *val) {
  gcvt( val->real, 15, buf );
}

/*----------------------------------------------------------------
  Type Definition Structures
----------------------------------------------------------------*/

/* A string is a standard dynamically-allocated string */
RtgPropEltTypeDef pet_string = {
  string_assign,
  string_2pict,
  elt_2string,
  string_compare,
  NULL,
  NULL,
  ascii_2string, 
  string_2ascii
};

/* a key string is a string which references a ChanTree */
RtgPropEltTypeDef pet_key_string = {
  string_assign,
  string_2pict,
  elt_2string,
  string_compare,
  key_check,
  key_recover,
  ascii_2string,
  string_2ascii
};

/* booleans use the boolean member */
RtgPropEltTypeDef pet_boolean = {
  bool_assign,
  bool_2pict,
  elt_2bool,
  bool_compare,
  NULL,
  NULL,
  ascii_2bool,
  bool_2ascii
};

/* exclusives use the boolean member */
RtgPropEltTypeDef pet_exclusive = {
  bool_assign,
  excl_2pict,
  elt_2excl,
  bool_compare,
  NULL,
  NULL,
  ascii_2bool,
  bool_2ascii
};

/* numus uses ushort_int member */
RtgPropEltTypeDef pet_numus = {
  numus_assign,
  numus_2pict,
  elt_2numus,
  numus_compare,
  NULL, /* check */
  NULL, /* recover */
  ascii_2numus,
  numus_2ascii
};

/* numreal uses real member */
RtgPropEltTypeDef pet_numreal = {
  real_assign,
  numreal_2pict,
  elt_2numreal,
  real_compare,
  NULL, /* check */
  NULL, /* recover */
  ascii_2real,
  real_2ascii
};

/* textreal uses real member */
RtgPropEltTypeDef pet_textreal = {
  real_assign,
  textreal_2pict,
  elt_2textreal,
  real_compare,
  NULL, /* check */
  NULL, /* recover */
  ascii_2real,
  real_2ascii
};

/* nop does nothing! */
RtgPropEltTypeDef pet_nop = {
  NULL, /* assign */
  NULL, /* 2 pict */
  NULL, /* elt 2 val */
  NULL, /* compare */
  NULL, /* check */
  NULL, /* recover */
  NULL, /* ascii 2 val */
  NULL  /* val 2 ascii */
};
