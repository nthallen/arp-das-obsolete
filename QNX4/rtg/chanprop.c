/* chanprop.c handles channel properties menus.
 * This is work very much in progress. I would like to have the
 * nested properties handled entirely by the dialog manager, but
 * that doesn't seem to work at present, so I will have to handle
 * dispatching of the sub-dialogs.
 
 Present Strategy:
 I will allow only one instance of the channel properties dialog.
 A second invocation will instead update the first. Correspondingly,
 I should allow only one instance each of X and Y axis properties.
 This won't work, since I need to access axis props from graph
 props also.
*/
#include <windows/Qwindows.h>
#include "rtg.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

static RtgPropEltDef chanp_elts[] = {
  "APN", &pet_key_string, offsetof(chandef, name), /* name */
  "APX", &pet_string, offsetof(chandef, units.X), /* X Units */
  "APY", &pet_string, offsetof(chandef, units.Y), /* Y Units */
  NULL,    0,               NULL
};

RtgPropDefA chanpropdef = {
  SRCDIR "chanprop.pict", /* filename of the dialog picture */
  "$chanprop",            /* The picture name, beginning with '$' */
  "rCP",                 /* The dialog label, beginning with 'r' */
  "Channel Properties",  /* The dialog title */
  NULL,                  /* find_prop method */
  CT_CHANNEL,            /* ChanTree to search */
  NULL,                  /* dial_update method */
  NULL,                  /* handler method */
  NULL,                  /* apply method */
  NULL,                  /* applied method */
  NULL,                  /* cancel method */
  chanp_elts             /* element definitions */
};
