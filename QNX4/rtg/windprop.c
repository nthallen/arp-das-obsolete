/* windprop.c handles window properties using the "proper"
   parametric property dialog system
 * $Log$
 */
#include <string.h>
#include <windows/Qwindows.h>
#include "rtg.h"

#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static RtgPropEltDef winp_elts[] = {
  "APSbT", offsetof(BaseWin, title), /* Window Title */
  "APBt",  offsetof(BaseWin, title_bar), /* Title Bar */
  NULL, 0
};

static int winp_apply(RtgPropDefB *PDB) {
  RtgPropDefA *pd;
  BaseWin *bw;
  
  pd = PDB->def;
  bw = (BaseWin *)PDB->prop_ptr;
  if (strcmp(bw->title, PDB->newvals[0].text) != 0) {
	WindowCurrent(bw->wind_id);
	WindowMsg(PDB->newvals[0].text, QW_KEEP, QW_KEEP, QW_KEEP);
  }
  if (bw->title_bar != PDB->newvals[1].boolean) {
	QW_RECT_AREA warea;
	
	if (bw->wind_id != 0) {
	  WindowCurrent(bw->wind_id);
	  WindowInfo(NULL, &warea, NULL, NULL, NULL, NULL);
	  bw->row = warea.row;
	  bw->col = warea.col;
	  basewin_close(bw);
	}
  }
  return 1;
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
  NULL,                  /* cancel method */
  winp_elts              /* element definitions */
};
