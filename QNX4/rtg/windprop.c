/* windprop.c handles window properties using the "proper"
   parametric property dialog system
 * $Log$
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
  NULL,    0,               NULL
};

static int winp_apply(RtgPropDefB *PDB) {
  BaseWin *bw;
  
  bw = (BaseWin *)PDB->prop_ptr;
  if (PDB->newvals[0].changed) {
	WindowCurrent(bw->wind_id);
	WindowMsg(PDB->newvals[0].val.text, QW_KEEP, QW_KEEP, QW_KEEP);
  }
  if (PDB->newvals[1].changed) {
	QW_RECT_AREA warea;
	
	if (bw->wind_id != 0) {
	  WindowCurrent(bw->wind_id);
	  WindowInfo(NULL, &warea, NULL, NULL, NULL, NULL);
	  bw->row = warea.row;
	  bw->col = warea.col;
	  basewin_close(bw);
	}
  } else if (PDB->newvals[2].changed) {
	char *ff_opt;
	ff_opt = PDB->newvals[2].val.boolean ? "f" : "-f";
	WindowCurrent(bw->wind_id);
	WindowChange(QW_KEEP, 0, 0, ff_opt, QW_KEEP);
  }
  return 1;
}

static void winp_applied(RtgPropDefB *PDB) {
  BaseWin *bw;
  
  bw = (BaseWin *)PDB->prop_ptr;
  if (PDB->newvals[0].changed) {
	BaseWin *bw;
	RtgAxis *ax;
  
	bw = (BaseWin *)PDB->prop_ptr;
	for (ax = bw->x_axes; ax != 0; ax = ax->next)
	  axis_ctname(ax);
	for (ax = bw->y_axes; ax != 0; ax = ax->next)
	  axis_ctname(ax);
  }
}

RtgPropDefA winpropdef = {
  SRCDIR "windprop.pict", /* filename of the dialog picture */
  "$windprop",            /* The picture name, beginning with '$' */
  "rWP",                 /* The dialog label, beginning with 'r' */
  "Window Properties",   /* The dialog title */
  NULL,                  /* find_prop method */
  CT_WINDOW,             /* ChanTree to search */
  NULL,                  /* dial_update method */
  NULL,                  /* handler method */
  winp_apply,            /* apply method */
  winp_applied,          /* applied method */
  NULL,                  /* cancel method */
  winp_elts              /* element definitions */
};
