/* windprop.c handles window properties using the "proper"
   parametric property dialog system
 * $Log$
 * Revision 1.3  1995/02/14  15:17:18  nort
 * Halfway through scripting
 *
 * Revision 1.2  1995/01/27  20:35:00  nort
 * *** empty log message ***
 *
 * Revision 1.1  1995/01/05  19:03:01  nort
 * Initial revision
 *
 */
#include <string.h>
#include <windows/Qwindows.h>
#include "rtg.h"

#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static RtgPropEltDef winp_elts[] = {
  "APSbT", &pet_key_string, offsetof(BaseWin, title), /* Window Title */
  "APBt",  &pet_boolean,    offsetof(BaseWin, title_bar), /* Title Bar */
  "APF",   &pet_boolean,    offsetof(BaseWin, fix_front),
  "apR",   &pet_numus,      offsetof(BaseWin, row),
  "apC",   &pet_numus,      offsetof(BaseWin, col),
  "apW",   &pet_numus,      offsetof(BaseWin, width),
  "apH",   &pet_numus,      offsetof(BaseWin, height),
  NULL,    0,               NULL
};

static void winp_applied(RtgPropDefB *PDB) {
  BaseWin *bw;
  
  bw = (BaseWin *)PDB->prop_ptr;
  if (PDB->newvals[1].changed) { /* Show title bar */
	Basewin_record( bw );
	basewin_close( bw );
  } else {
	if (PDB->newvals[0].changed) { /* title */
	  WindowCurrent(bw->wind_id);
	  WindowMsg(PDB->newvals[0].val.text, QW_KEEP, QW_KEEP, QW_KEEP);
	}
	if (PDB->newvals[2].changed) { /* keep front */
	  char *ff_opt;
	  ff_opt = PDB->newvals[2].val.boolean ? "f" : "-f";
	  WindowCurrent(bw->wind_id);
	  WindowChange(QW_KEEP, 0, 0, ff_opt, QW_KEEP);
	}
  }
}

RtgPropDefA winpropdef = {
  "windprop.pict",       /* filename of the dialog picture */
  "$windprop",           /* The picture name, beginning with '$' */
  "rWP",                 /* The dialog label, beginning with 'r' */
  "Window Properties",   /* The dialog title */
  NULL,                  /* find_prop method */
  CT_WINDOW,             /* ChanTree to search */
  NULL,                  /* dial_update method */
  NULL,                  /* handler method */
  NULL,                  /* apply method */
  winp_applied,          /* applied method */
  NULL,                  /* cancel method */
  winp_elts              /* element definitions */
};
