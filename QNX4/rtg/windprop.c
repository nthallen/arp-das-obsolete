/* windprop.c handles window properties using the "proper"
   parametric property dialog system
 * $Log$
 * Revision 1.4  1995/02/14  21:05:42  nort
 * Scripting is Working
 *
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
  "APD",   &pet_boolean,    offsetof(BaseWin, draw_direct),
  "APC",   &pet_numus,      offsetof(BaseWin, bkgd_color),
  "APP",   &pet_numus,      offsetof(BaseWin, bkgd_pattern),
  "apR",   &pet_numus,      offsetof(BaseWin, row),
  "apC",   &pet_numus,      offsetof(BaseWin, col),
  "apW",   &pet_numus,      offsetof(BaseWin, width),
  "apH",   &pet_numus,      offsetof(BaseWin, height),
  NULL,    0,               NULL
};
static enum eltnum { EN_TITLE, EN_TITLE_BAR, EN_FIX_FRONT, EN_DRAW_DIRECT,
	EN_BKGD_COLOR, EN_BKGD_PAT, EN_ROW, EN_COL, EN_WIDTH, EN_HEIGHT };

static void winp_apply( RtgPropDefB *PDB ) {
  if ( PDB->newvals[ EN_DRAW_DIRECT ].changed ) {
	BaseWin *bw;
  
	bw = (BaseWin *)PDB->prop_ptr;
	basewin_erase( bw );
  }
}

static void winp_applied(RtgPropDefB *PDB) {
  BaseWin *bw;
  
  bw = (BaseWin *)PDB->prop_ptr;
  if (PDB->newvals[ EN_TITLE_BAR ].changed) { /* Show title bar */
	Basewin_record( bw );
	basewin_close( bw );
  } else {
	if (PDB->newvals[ EN_TITLE ].changed) { /* title */
	  WindowCurrent( bw->wind_id );
	  WindowMsg( bw->title, QW_KEEP, QW_KEEP, QW_KEEP);
	  WindowIcon( NULL, bw->title, QW_KEEP, QW_KEEP, 
				  QW_TRANSPARENT );
	}
	if (PDB->newvals[ EN_FIX_FRONT ].changed) { /* keep front */
	  char *ff_opt;
	  ff_opt = bw->fix_front ? "f" : "-f";
	  WindowCurrent(bw->wind_id);
	  WindowChange(QW_KEEP, 0, 0, ff_opt, QW_KEEP);
	}
  }
  if ( PDB->newvals[ EN_BKGD_COLOR ].changed ||
		PDB->newvals[ EN_BKGD_PAT ].changed ) {
	PictureCurrent( bw->pict_id );
	PictureChange( QW_KEEP, bw->bkgd_color, bw->bkgd_pattern, QW_KEEP,
			  QW_KEEP );
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
  winp_apply,            /* apply method */
  winp_applied,          /* applied method */
  NULL,                  /* cancel method */
  winp_elts              /* element definitions */
};
