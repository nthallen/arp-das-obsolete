/* graphprop.c Perhaps a more general-purpose properties dialog.
*/
#include <string.h>
#include <assert.h>
#include <windows/Qwindows.h>
#include "rtg.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

/* graphprop.pict contains (some of) the following elements:

   Tag   Description
   ---   ------------------------
   APn   Graph Name
   APc   Graph Color (number, 0-16)
   APt   Graph Thickness (number, 0-5, to be multiplied by QW_V_TPP)
   APs   Graph Line Style (number)
   APS   Graph Symbol (text...)
   APC   Graph Symbol Color (number)
     Other elements:
   Apc   Channel Name
   x     X Axis Selection
   y     Y Axis Selection
   P*    Graphical and otherwise non-interactive elements
     Other messages
*/
static RtgPropEltDef grfp_elts[] = {
  "APn", &pet_key_string, offsetof(RtgGraph, name),
  "APc", &pet_numus, offsetof(RtgGraph, line_color),
  "APt", &pet_numus, offsetof(RtgGraph, line_thickness),
  "APs", &pet_numus, offsetof(RtgGraph, line_style),
  "APS", &pet_string, offsetof(RtgGraph, symbol),
  "APC", &pet_numus, offsetof(RtgGraph, symbol_color),
  NULL,    0,               NULL
};
enum eltnum {EN_NAME, EN_LN_COL, EN_LN_TH, EN_LN_STY, EN_SYM,
	EN_SYM_COL };

/* The only non-standard thing we have to do is update the channel name,
   which is read/only on the screen
 */
int grf_dial_update(RtgPropDefB *PDB) {
  RtgGraph *graph;
  
  graph = (RtgGraph *)PDB->prop_ptr;
  ChangeText("Apc", graph->position->channel->name, 0, -1, 0);
  PropUpdate_(graph->X_Axis->opt.ctname, "XP");
  PropUpdate_(graph->Y_Axis->opt.ctname, "YP");
  return 0;
}

int grf_handler(QW_EVENT_MSG *msg, RtgPropDefB *PDB) {
  RtgGraph *graph;
  
  graph = (RtgGraph *)PDB->prop_ptr;
  switch (msg->hdr.key[0]) {
	case 'x':
	  Properties_( graph->X_Axis->opt.ctname, "XP", 1 );
	  break;
	case 'y':
	  Properties_( graph->Y_Axis->opt.ctname, "YP", 1 );
	  break;
	default:
	  return 0;
  }
  return 1;
}

static int grf_apply(RtgPropDefB *PDB) {
  RtgGraph *graph;
  
  graph = (RtgGraph *)PDB->prop_ptr;
  if (PDB->newvals[EN_LN_COL].changed ||
	  PDB->newvals[EN_LN_TH].changed ||
	  PDB->newvals[EN_LN_STY].changed ||
	  PDB->newvals[EN_SYM].changed ||
	  PDB->newvals[EN_SYM_COL].changed
  ) {
	RtgGraph *graph;
  
	graph = (RtgGraph *)PDB->prop_ptr;
	graph->window->redraw_required = 1;
  }
  PropCancel_( graph->X_Axis->opt.ctname, "XP", NULL );
  PropCancel_( graph->Y_Axis->opt.ctname, "YP", NULL );
  return 1;
}

static int grf_cancel(RtgPropDefB *PDB) {
  RtgGraph *graph;
  
  graph = (RtgGraph *)PDB->prop_ptr;
  PropCancel_( graph->X_Axis->opt.ctname, "XP", NULL );
  PropCancel_( graph->Y_Axis->opt.ctname, "YP", NULL );
  return 1;
}

RtgPropDefA grfpropdef = {
  "graphprop.pict",      /* filename of the dialog picture */
  "$graphprop",          /* The picture name, beginning with '$' */
  "rGP",                 /* The dialog label, beginning with 'r' */
  "Graph Properties",    /* The dialog title */
  NULL,                  /* find_prop method */
  CT_GRAPH,              /* ChanTree to search */
  grf_dial_update,       /* dial_update method */
  grf_handler,           /* handler method */
  grf_apply,             /* apply method */
  NULL,                  /* applied method */
  grf_cancel,            /* cancel method */
  grfp_elts              /* element definitions */
};
