/* axisprop.c 3rd rewrite...
*/
#include <windows/Qwindows.h>
#include "rtg.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static RtgPropEltDef axp_elts[] = {
  "APU", &pet_string, offsetof(RtgAxis, opt.units), /* units */
  "APLm", &pet_numreal, offsetof(RtgAxis, opt.limits.min),
  "APLM", &pet_numreal, offsetof(RtgAxis, opt.limits.max),
  "APAm", &pet_boolean, offsetof(RtgAxis, opt.min_auto),
  "APAM", &pet_boolean, offsetof(RtgAxis, opt.max_auto),
  "APXW", &pet_boolean, offsetof(RtgAxis, opt.single_sweep),
  "APXC", &pet_boolean, offsetof(RtgAxis, opt.clear_on_trig),
  "APYO", &pet_boolean, offsetof(RtgAxis, opt.overlay),
  "APXS0", &pet_boolean, offsetof(RtgAxis, opt.scope),
  "APXS1", &pet_boolean, offsetof(RtgAxis, opt.scroll),
  "APXS", &pet_nop, 0,
  "APXN", &pet_exclusive, offsetof(RtgAxis, opt.normal),
  "APYW", &pet_numus, offsetof(RtgAxis, opt.weight),
  NULL,    0,               NULL
};
enum eltnum {EN_UNITS, EN_MIN, EN_MAX, EN_MINA, EN_MAXA,
  EN_SS, EN_COT, EN_OVERLAY, EN_SCOPE, EN_SCROLL, EN_NORMAL, EN_WEIGHT};

static int axp_apply(RtgPropDefB *PDB) {
  RtgAxis *Axis;
  
  Axis = (RtgAxis *)PDB->prop_ptr;

  /* If limits have changed, rescale is required */
  if (PDB->newvals[EN_MIN].changed || PDB->newvals[EN_MAX].changed)
	Axis->rescale_required = 1;

  if (PDB->newvals[EN_WEIGHT].changed || PDB->newvals[EN_OVERLAY].changed)
	Axis->window->resize_required = 1;

  /* I don't think any of these require action here:
	  min_auto
	  max_auto
	  scope
	  scroll
	  normal
	  single_sweep
	  clear_on_trig
  */
  return 1;
}

static void axp_applied(RtgPropDefB *PDB) {
  RtgAxis *Axis;
  
  Axis = (RtgAxis *)PDB->prop_ptr;

  if (PDB->newvals[EN_UNITS].changed)
	axis_ctname(Axis);
}

RtgPropDefA x_axpropdef = {
  SRCDIR "axprop2.pict", /* filename of the dialog picture */
  "$xaxprop",            /* The picture name, beginning with '$' */
  "rXP",                 /* The dialog label, beginning with 'r' */
  "X Axis Properties",   /* The dialog title */
  NULL,                  /* find_prop method */
  CT_AXIS,               /* ChanTree to search */
  NULL,                  /* dial_update method */
  NULL,                  /* handler method */
  axp_apply,             /* apply method */
  axp_applied,           /* applied method */
  NULL,                  /* cancel method */
  axp_elts               /* element definitions */
};

RtgPropDefA y_axpropdef = {
  SRCDIR "axprop2.pict", /* filename of the dialog picture */
  "$yaxprop",            /* The picture name, beginning with '$' */
  "rYP",                 /* The dialog label, beginning with 'r' */
  "Y Axis Properties",   /* The dialog title */
  NULL,                  /* find_prop method */
  CT_AXIS,               /* ChanTree to search */
  NULL,                  /* dial_update method */
  NULL,                  /* handler method */
  axp_apply,             /* apply method */
  axp_applied,           /* applied method */
  NULL,                  /* cancel method */
  axp_elts               /* element definitions */
};
