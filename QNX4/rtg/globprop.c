/* globprop.c Handles Global Properties
 * $Log$
 */
#include <stddef.h>
#include <windows/Qwindows.h>
#include "rtg.h"

#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static RtgPropEltDef globp_elts[] = {
  "APC", &pet_string, offsetof(RtgGlobOpt, config_file),
  NULL,    0,               NULL
};

static void *globp_find(const char *name, RtgPropDefB *prop_def) {
  name = name;
  prop_def = prop_def;
  return &GlobOpts;
}

RtgPropDefA globpropdef = {
  "globprop.pict",       /* filename of the dialog picture */
  "$globprop",           /* The picture name, beginning with '$' */
  "rRP",                 /* The dialog label, beginning with 'r' */
  "Global Properties",   /* The dialog title */
  globp_find,            /* find_prop method */
  0,                     /* ChanTree to search */
  NULL,                  /* dial_update method */
  NULL,                  /* handler method */
  NULL,                  /* apply method */
  NULL,                  /* applied method */
  NULL,                  /* cancel method */
  globp_elts             /* element definitions */
};
