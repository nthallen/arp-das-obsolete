/* graphprop.c Perhaps a more general-purpose properties dialog.
*/
#include <string.h>
#include <assert.h>
#include <windows/Qwindows.h>
#include "rtg.h"
#include "nortlib.h"
/* graphprop.pict contains (some of) the following elements:

   Tag   Description
   ---   ------------------------
   APn   Graph Name
   APc   Graph Color (number, 0-16)
   APt   Graph Thickness (number, 0-5, to be multiplied by QW_V_TPP)
   APs   Graph Line Style (number)
   APS   Graph Symbol (text...)
   APC   Graph Symbol Color (number)
   Apc   Channel Name
   x     X Axis Selection
   y     Y Axis Selection
     Other elements:
   P*    Graphical and otherwise non-interactive elements
     Other messages
   APN   No Op
*/

static RtgGraph *prop_graph, new_opts;

/* prop2dial copies appropriate prop information into the
   dialog. It assumes the picture is current. The proptype
   is included for dialogs which share this function with
   other dialogs (axis props, for example). Other such
   functions will not need to use that arg.
   If name is NULL, dialog should be updated with the
   properties from the current object.
   Returns 0 on success, 1 on failure. May call nl_error.
*/
static int gr_prop2dial(const char *name, enum proptypes unrefd) {
  RtgChanNode *CN;

  if (name == 0) {
	assert(prop_graph != 0);
  } else {
	CN = ChanTree(CT_FIND, CT_GRAPH, name);
	assert(CN != 0 && CN->u.leaf.graph != 0);
	prop_graph = CN->u.leaf.graph;
	PropUpdate(prop_graph->name, GR_X_PROPS);
	PropUpdate(prop_graph->name, GR_Y_PROPS);
  }

  dastring_update(&new_opts.name, prop_graph->name);
  ChangeText("APn", new_opts.name, 0, -1, 0);
  ChangeText("Apc", prop_graph->position->channel->name, 0, -1, 0);

  new_opts.line_thickness = prop_graph->line_thickness;
  ChangeNumber("APt", new_opts.line_thickness);

  new_opts.line_color = prop_graph->line_color;
  ChangeNumber("APc", new_opts.line_color);

  new_opts.line_style = prop_graph->line_style;
  ChangeNumber("APs", new_opts.line_style);

  new_opts.symbol[0] = prop_graph->symbol[0];
  new_opts.symbol[1] = '\0';
  ChangeText("APS", new_opts.symbol, 0, -1, 0);

  new_opts.symbol_color = prop_graph->symbol_color;
  ChangeNumber("APC", new_opts.symbol_color);
  
  return 0;
}

/* dial2prop updates the new properties structure based on
   the single element tag. The tagged element is the Current
   element, so ElementNumber(), ElementText() etc. will
   return the appropriate value. As before, proptype
   may be ignored.
*/
static int gr_dial2prop(char *tag, enum proptypes unrefd) {
  switch (tag[2]) {
	case 'n': /* Graph Name */
	  dastring_update(&new_opts.name, ElementText()); break;
	case 'c': /* Line Color */
	  new_opts.line_color = ElementNumber(); break;
	case 't': /* Line Thickness */
	  new_opts.line_thickness = ElementNumber(); break;
	case 's': /* Line Style */
	  new_opts.line_style = ElementNumber(); break;
	case 'S': /* Symbol */
	  { char *text;
		text = ElementText();
		new_opts.symbol[0] = (*text == ' ') ? 0 : *text;
		new_opts.symbol[1] = '\0';
	  }
	  break;
	case 'C': /* Symbol Color */
	  new_opts.symbol_color = ElementNumber(); break;
	default:
	  nl_error(2, "Unrecognized tag %s in graph props", tag);
	  break;
  }
  return 0;
}

/* calling this function indicates that all the elements
   have been processed and the new values should be copied
   to the actual properties structure. In many cases,
   this is not strictly necessary, since the values can
   be safely copied in the dial2prop function, but this
   allows for global sanity checks before applying the
   results. A zero result indicates that the values
   have not been applied (presumably an error was reported
   via nl_error(2)) and the dialog should not be cancelled.
*/
static int gr_apply(enum proptypes unrefd) {
  assert(prop_graph != 0);
  if (strcmp(prop_graph->name, new_opts.name) != 0) {
	if (ChanTree_Rename(CT_GRAPH, prop_graph->name, new_opts.name))
	  dastring_update(&prop_graph->name, new_opts.name);
	else {
	  nl_error(2, "Unable to rename graph");
	  return 0;
	}
  }
  if (prop_graph->line_color != new_opts.line_color ||
	  prop_graph->line_thickness != new_opts.line_thickness ||
	  prop_graph->line_style != new_opts.line_style ||
	  prop_graph->symbol[0] != new_opts.symbol[0] ||
	  prop_graph->symbol_color != new_opts.symbol_color) {
	prop_graph->window->redraw_required = 1;
  }
  prop_graph->line_color = new_opts.line_color;
  prop_graph->line_thickness = new_opts.line_thickness;
  prop_graph->line_style = new_opts.line_style;
  prop_graph->symbol[0] = new_opts.symbol[0];
  prop_graph->symbol[1] = '\0';
  prop_graph->symbol_color = new_opts.symbol_color;
  return 1;
}

/* Auxilliary handler routine. QW_DISMISS and key 'Y'
   are handled first (so not passed to handler)
   Returns 1 if the message is handled, 0 otherwise.
   QW_CLOSED and QW_CANCELLED are handled afterward
   if handler doesn't. May be NULL.
*/
int gr_handler(QW_EVENT_MSG *msg, enum proptypes unrefd) {
  switch (msg->hdr.key[0]) {
	case 'x':
	  Properties(prop_graph->name, GR_X_PROPS);
	  break;
	case 'y':
	  Properties(prop_graph->name, GR_Y_PROPS);
	  break;
	default:
	  return 0;
  }
  return 1;
}

/* Called when the dialog is to be cancelled, allows
   other actions to be taken (such as cancelling nested
   dialogs). May return 0 if the specified name and
   proptype do not match the currently open dialog.
   (e.g. cancel for graph props will be called whenever
   a graph is deleted. Graph props for another graph
   may be open, but that shouldn't be cancelled)
   Returns non-zero if the name and proptype match the
   currently active dialog. May be NULL if no test is
   required. The specific dialog in question does not
   need to be cancelled by this routine.
*/
int gr_cancel(const char *name, enum proptypes unrefd) {
  RtgChanNode *CN;
  
  CN = ChanTree(CT_FIND, CT_GRAPH, name);
  if (CN == 0)
	nl_error(1, "Internal, CN==0 in graphprop.c at %d", __LINE__);
  else if (CN->u.leaf.graph == prop_graph) {
	prop_graph = NULL;
	PropCancel(NULL, GR_X_PROPS);
	PropCancel(NULL, GR_Y_PROPS);
	return 1;
  }
  return 0;
}

RtgPropDef graph_props = {
  SRCDIR "graphprop.pict", 0, "$grprop", "pGP", "Graph Properties",
  gr_prop2dial, gr_dial2prop, gr_apply, gr_handler, gr_cancel
};
