/* axisprop.c Perhaps a more general-purpose properties dialog.
*/
#include <string.h>
#include <assert.h>
#include <windows/Qwindows.h>
#include "rtg.h"
#include "nortlib.h"

/*
  Must support (at least) both channel/axis properties and
  graph/axis properties. May also need to support channel-type/axis,
  etc.

  Must record source of axis data in order to process "Apply" and
  "Reset". Should probably display the source info: Channel Counts X
  Graph Counts Y.

  Since I only support one channel-props dialog, I need only support
  one channel/X and one channel/Y dialog. Probably the same for the
  graphs. A subsequent invocation of the same type indicates a selection
  at a higher level, requiring updating of the current selections.
  
  Updating a channel's axis options is a simple copy (axopts_update())
  Updating a graph's axis options requires considerably more care...
*/

/* axisprop.pict must contain the following elements:
   Tag   Description
   APU   Units (text)
   APLm  Min Limit (Real)
   APLM  Max Limit (Real)
   APOm  Min Observed (Real)
   APOM  Max Observed (Real)
   APAm  Auto Scale Minimum (Check box)
   APAM  Auto Scale Maximum (Check box)
   APXS  Scope(0)/Scroll(1) (Independent Settings)
   APXN  Normal(0)/Auto(1) (Exclusive Settings)
   APXW  Single Sweep (Check Box)
   APXC  Clear on Trigger (Check Box)
   APYO  Overlay (Check Box)
   APYF  Force New (Check Box)
   APYW  Weight (Integer)
     Other elements:
   P*    Graphical and otherwise non-interactive elements
     Other messages
   APN   No Op
*/

typedef struct {
  int pict_id;
  const char *name;
} axprop;

static axprop axprops[AP_NTYPES];

typedef struct {
  char *label; /* Label for the dialog */
  char *pname; /* The name of the picture */
  char *title; /* Title for the window */
} axname;

static axname axnames[AP_NTYPES] = {
  "AP0", "$ACX", "Channel X Axis",
  "AP1", "$ACY", "Channel Y Axis",
  "AP2", "$AGX", "Graph X Axis",
  "AP3", "$AGY", "Graph Y Axis"
};

static int saved_pict;
#define AP_PICT SRCDIR "axisprop.pict"

/* Standard Property Dialog buttons: */
static char *bars[4] = { NULL, "Apply|Y;Reset|CANCEL", NULL, NULL };

static int label2type(char *label) {
  if (label[0] == 'A' && label[1] == 'P')
	return label[2]-'0';
  else return -1;
}

static RtgAxisOpts *type2opts(enum axprop_type type) {
  RtgAxisOpts *opts;

  assert(type >= 0 && type < AP_NTYPES);
  if (type <= AP_CHANNEL_Y) {
	chandef *channel;
	channel = channel_props(axprops[type].name);
	assert(channel != 0);
	if (type == AP_CHANNEL_X) opts = &channel->opts.X;
	else opts = &channel->opts.Y;
  } else {
	Tell("ap_update", "Graphs not yet supported");
	return NULL;
  }
  return opts;
}

/* This is where we actually make axis properties changes
   This routine will not be responsible for cancelling the
   dialog, but can assume that the dialog is still active.
   (which is a good thing, since we'll need it to retrieve the
   values...) I will extract the 
 */
#define ELTBUF_SIZE 4096
static void ap_apply(char *label) {
  int type;
  RtgAxisOpts *opts, newopt;
  void *eltbuf, *elt;
  axprop *ap;
  
  type = label2type(label);
  assert(type >= 0 && type < AP_NTYPES);
  ap = &axprops[type];
  eltbuf = new_memory(ELTBUF_SIZE);
  PictureCurrent(ap->pict_id);
  CopyElements("AP*", eltbuf, ELTBUF_SIZE, NULL);
  for (elt = ElementFirst(eltbuf); elt != 0; elt = ElementNext()) {
	char *tag;
	int eltltr;
	
	tag = ElementTag();
	assert(tag[0] == 'A' && tag[1] == 'P');
	eltltr = tag[2];
	if (tag[3] != 0) eltltr = (eltltr << 8) | tag[3];
	switch(eltltr) {
	  case 'U':   /*  Units (text) */
		newopt.units = ElementText(); break;
	  case 'Lm':  /*  Min Limit (Real) */
		newopt.limits.min = ElementReal(); break;
	  case 'LM':  /*  Max Limit (Real) */
		newopt.limits.max = ElementReal(); break;
	  case 'Om':  /*  Min Observed (Real) */
		newopt.obsrvd.min = ElementReal(); break;
	  case 'OM':  /*  Max Observed (Real) */
		newopt.obsrvd.max = ElementReal(); break;
	  case 'Am':  /*  Auto Scale Minimum (Check box) */
		newopt.min_auto = ElementState(); break;
	  case 'AM':  /*  Auto Scale Maximum (Check box) */
		newopt.max_auto = ElementState(); break;
	  case 'XS':  /*  Scope(0)/Scroll(1) (Independent Settings) */
		switch (tag[4]) {
		  case '\0': break;
		  case '0': newopt.scope = ElementState(); break;
		  case '1': newopt.scroll = ElementState(); break;
		  default: Tell("ap_apply", "Unexpected tag %s", tag); break;
		}
		break;
	  case 'XN':  /*  Normal(0)/Auto(1) (Exclusive Settings) */
		switch (tag[4]) {
		  case '\0': break;
		  case '0': newopt.normal = ElementState(); break;
		  case '1': break;
		  default: Tell("ap_apply", "Unexpected tag %s", tag); break;
		}
		break;
	  case 'XW':  /*  Single Sweep (Check Box) */
		newopt.single_sweep = ElementState(); break;
	  case 'XC':  /*  Clear on Trigger (Check Box) */
		newopt.clear_on_trig = ElementState(); break;
	  case 'YO':  /*  Overlay (Check Box) */
		newopt.overlay = ElementState(); break;
	  case 'YF':  /*  Force New (Check Box) */
		newopt.force_new = ElementState(); break;
	  case 'YW':  /*  Weight (Integer) */
		newopt.weight = ElementNumber(); break;
	  default:
		Tell("ap_apply", "Unexpected tag %s", tag); break;
	}
  }
  opts = type2opts(type);
  assert(opts != 0);
  if (type <= AP_CHANNEL_Y)
	axopts_update(opts, &newopt);
  else Tell("ap_apply", "Graph Axis Updates Not Implemented");
  free_memory(eltbuf);
}

static int axprop_handler(QW_EVENT_MSG *msg, char *label) {
  if (msg->hdr.action == QW_DISMISS) {
	int i;

    i = label2type(msg->hdr.key);
	if (i >= 0 && i < AP_NTYPES)
	  axisprop_delete(i);
	else
	  EventNotice("AP Handler Dismiss", msg);
	return 1;
  }
  switch (msg->hdr.key[0]) {
	case 'Y': /* Apply */
	  ap_apply(label);
	  DialogCancel(label, NULL);
	  break;
	case 'N': /* No op */
	  break;
	default:
	  switch (msg->hdr.action) {
		case QW_CLOSED:
		case QW_CANCELLED:
		  break;
		default:
		  EventNotice("AxisProp", msg);
	  }
	  break;
  }
  return 1;
}

static void init_handler(void) {
  static int handler_set = 0;
  
  if (!handler_set) {
	set_key_handler('A', axprop_handler);
	handler_set = 1;
  }
}

static void ap_update(enum axprop_type type) {
  axprop *ap;
  RtgAxisOpts *opts;
  
  assert(type >= 0 && type < AP_NTYPES);
  ap = &axprops[type];
  assert(ap->name != 0);
  opts = type2opts(type);
  if (opts == 0) return;
  PictureCurrent(ap->pict_id);
  ChangeText("APU", opts->units, 0, -1, 0);
  ChangeReal("APLm", opts->limits.min);
  ChangeReal("APLM", opts->limits.max);
  ChangeReal("APOm", opts->obsrvd.min);
  ChangeReal("APOM", opts->obsrvd.max);
  ChangeState("APAm", opts->min_auto ? 1 : 0);
  ChangeState("APAM", opts->max_auto ? 1 : 0);
  ChangeState("APXN*", 0);
  if (type & 1) { /* Y Axes */
	ChangeOptions("APX*", "d-S");
	ChangeOptions("APY*", "S-d");
	ChangeState("APYO", opts->overlay ? 1 : 0);
	ChangeState("APYF", opts->force_new ? 1 : 0);
	ChangeNumber("APYW", opts->weight);
  } else { /* X Axes */
	ChangeOptions("APY*", "d-S");
	ChangeOptions("APX*", "S-d");
	ChangeState("APXS0", opts->scope ? 1 : 0);
	ChangeState("APXS1", opts->scroll ? 1 : 0);
	if (opts->normal) ChangeState("APXN0", 1);
	else ChangeState("APXN1", 1);
	ChangeState("APXW", opts->single_sweep ? 1 : 0);
	ChangeState("APXC", opts->clear_on_trig ? 1 : 0);
  }
}

void axisprop_dialog(enum axprop_type type, const char *name) {
  axprop *ap;
  axname *apn;

  assert(type >= 0 && type < AP_NTYPES);
  ap = &axprops[type];
  apn = &axnames[type];
  ap->name = name;
  if (ap->pict_id == 0) {
	if (saved_pict == 0) {
	  saved_pict = Picture("aptemplate", AP_PICT);
	  if (saved_pict == 0) {
		Tell("axisprop", "Unable to locate axis props picture");
		return;
	  }
	}
	ap->pict_id = PictureOpen(apn->pname+1, NULL, apn->title,
							0, 0, NULL, "R;PROP");
	assert(ap->pict_id != 0);
	PictureCopy(QW_ALL, saved_pict, 0, NULL, "AF");
  }
  ap_update(type);
  if (DialogCurrent(apn->label) != YES)
	Dialog(apn->label, apn->title, NULL, bars, apn->pname, "cp-b");
  init_handler();
}

/* Update the specified dialog to the new name iff dialog is active */
void axisprop_update(enum axprop_type type, const char *name) {
  axname *apn;

  assert(type >= 0 && type < AP_NTYPES);
  apn = &axnames[type];
  if (DialogCurrent(apn->label))
	axisprop_dialog(type, name);
}

/* Close the corresponding dialog if open */
void axisprop_delete(enum axprop_type type) {
  axprop *ap;
  int rv;
  
  assert(type >= 0 && type < AP_NTYPES);
  ap = &axprops[type];
  if (ap->pict_id != 0) {
	DialogCancel(axnames[type].label, "P");
	rv = PictureClose(ap->pict_id);
	assert(rv == YES);
	ap->pict_id = 0;
	ap->name = NULL;
  }
}
