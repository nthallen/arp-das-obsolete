/* elttype.c defines the various property element types
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <windows/Qwindows.h>
#include "rtg.h"
#include "nortlib.h"

/* String and Key functions */
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
  if (old->text == 0 && new->text == 0)
	return 0;
  if (old->text == 0 || new->text == 0)
	return 1;
  return strcmp(old->text, new->text);
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

/* A string is a standard dynamically-allocated string */
RtgPropEltTypeDef pet_string = {
  string_assign,
  string_2pict,
  elt_2string,
  string_compare,
  NULL,
  NULL,
  ascii_2string
};

/* a key string is a string which references a ChanTree */
RtgPropEltTypeDef pet_key_string = {
  string_assign,
  string_2pict,
  elt_2string,
  string_compare,
  key_check,
  key_recover,
  ascii_2string
};

/* booleans use the long_int member */
RtgPropEltTypeDef pet_boolean = {
  bool_assign,
  bool_2pict,
  elt_2bool,
  bool_compare,
  NULL,
  NULL,
  ascii_2bool
};
