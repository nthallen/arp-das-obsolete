/* axis.c functions pertaining to axes */
#include <assert.h>
#include <windows/Qwindows.h>
#include "math.h"
#include "rtg.h"
#include "nortlib.h"

static RtgAxesOpts hard_defaults = {
	{ 0, 0, -1, 0, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0 },    /* X Reset Opts */
    { 0, 0, -1, 0, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0 } };  /* Y Reset Opts */

void axis_ctname(RtgAxis *axis) {
  RtgChanNode *CN;
  char ctname[80], *newname;

  if (axis->ctname != 0)
	ChanTree(CT_DELETE, CT_AXIS, axis->ctname);
  if (axis->window == 0) {
	sprintf(ctname, "default/%s/generic", axis->is_y_axis ? "Y" : "X");
	CN = ChanTree(CT_INSERT, CT_AXIS, ctname);
	newname = ctname;
  } else {
	sprintf(ctname, "%s/%s/%s/%%d",
	  axis->is_y_axis ? "Y" : "X", axis->opt.units, axis->window->title);
	newname = ChanTreeWild(CT_AXIS, ctname);
	CN = ChanTree(CT_FIND, CT_AXIS, newname);
  }
  assert(CN != 0);
  CN->u.leaf.axis = axis;
  dastring_update(&axis->ctname, newname);
}

/* May return NULL on error */
RtgAxis *axis_create(BaseWin *bw, const char *units, int is_y_axis) {
  RtgAxis *axis, **ax;
  char ctname[80], *axtype;
  RtgAxisOpts *opts;

  axtype = is_y_axis ? "Y" : "X";
  if (units == 0)
	units = axtype;
  sprintf(ctname, "default/%s/generic", axtype);
  if (bw == 0) {
	RtgChanNode *CN;

	CN = ChanTree(CT_FIND, CT_AXIS, ctname);
	if (CN != 0) {
	  nl_error(1, "Attempted re-creation of axis %s", ctname);
	  return CN->u.leaf.axis;
	}
  }

  /* New axes must go at the END of the list */
  for (ax = (is_y_axis ? &bw->y_axes : &bw->x_axes);
	   *ax != 0;
	   ax = &(*ax)->next) {
	if ((*ax)->opt.scope || (*ax)->opt.scroll) {
	  nl_error(2, "Cannot create second axis with scope or scroll");
	  return NULL;
	}
  }

  axis = new_memory(sizeof(RtgAxis));
  axis->next = NULL;
  *ax = axis;
  
  axis->window = bw; /* Is this necessary? Yes, I think it is! */
  axis->ctname = NULL;
  axis->auto_scale_required = 1;
  /* I won't bother initializing min_coord, max_coord or the scaling functions,
     I'll just indicate that a rescale is required and that the window must
	 be resized also, which will cause all the appropriate actions
  */
  axis->rescale_required = 1;
  bw->resize_required = 1;
  axis->redraw_required = 1;
  axis->is_y_axis = is_y_axis;
  axis->deleted = 0;

  opts = NULL;
  if (bw != 0) { /* Look for the default */
	RtgChanNode *CN;
	CN = ChanTree(CT_FIND, CT_AXIS, ctname);
	if (CN != 0)
	  opts = &CN->u.leaf.axis->opt;
  }
  if (opts == 0)
	opts = is_y_axis ? &hard_defaults.Y : &hard_defaults.X;
  axopts_init(&axis->opt, opts);
  dastring_update(&axis->opt.units, units);

  if (axis->opt.scroll || axis->opt.scope)
	axis->opt.min_auto = axis->opt.max_auto = 1;

  /* Force auto-limits if none specified */
  if (axis->opt.limits.min > axis->opt.limits.max)
	axis->opt.min_auto = axis->opt.max_auto = 1;

  /* Install in the ChanTree */
  axis_ctname(axis);

  return axis;
}

/* axis delete unlinks the axis from the window's list of axes and
   then deletes any graphs attached to it.
*/
void axis_delete(RtgAxis *ax) {
  RtgAxis **xp;
  BaseWin *bw;
  RtgGraph *graph;

  if (ax == 0 || ax->deleted) return;
  PropCancel_(ax->ctname, ax->is_y_axis ? "YP" : "XP", "P");
  bw = ax->window;
  xp = (ax->is_y_axis ? &bw->y_axes : &bw->x_axes);
  for (; *xp != 0 && *xp != ax; xp = &(*xp)->next) ;
  assert(*xp != 0);
  *xp = ax->next;
  ax->deleted = 1;

  /* delete all graphs attached to this axis */
  for (;;) {
	for (graph = bw->graphs; graph != NULL; graph = graph->next) {
	  if (graph->X_Axis == ax || graph->Y_Axis == ax) break;
	}
	if (graph == NULL) break;
	graph_delete(graph);
  }

  bw->resize_required = 1;

  assert(ax->ctname != 0);
  ChanTree(CT_DELETE, CT_AXIS, ax->ctname);
  dastring_update(&ax->ctname, NULL);
  dastring_update(&ax->opt.units, NULL);
  free_memory(ax);
}

/* Determines Min and Max values: does not scale the window
   This function may return early if it has a lot of work
   to do. In that case, it won't clear auto_scale_required
   and hence will be called again.
 */
void axis_auto_range(RtgAxis *ax) {
  BaseWin *bw;
  RtgGraph *graph;
  RtgAxis *bx;
  int min_auto, max_auto;
  
  min_auto = ax->opt.min_auto;
  max_auto = ax->opt.max_auto;
  if (min_auto == 0 && max_auto == 0) {
	ax->auto_scale_required = 0;
	return;
  }
  bw = ax->window;

  /* Look ahead on each graph attached to this axis */
  for (graph = bw->graphs; graph != NULL; graph = graph->next) {
	bx = (ax->is_y_axis) ? graph->Y_Axis : graph->X_Axis;
	if ((ax == bx) && (graph->looked_ahead == 0)) {
	  lookahead(graph);
	  /* That's enough work for now... */
	  return;
	}
  }

  /* Extra code for scope triggering x-axes required here!!! */
  /* Scrolling code here:
	  Can't do shifting until axis is scaled.
  */
  if ((!ax->is_y_axis) && ax->opt.scroll) {
	if (ax->opt.obsrvd.max > ax->opt.limits.max) {
	  double dist;
	  int shift;
	
	  #define SHIFT_PCT .2
	  
	  /* lookahead should have anchored the base for us */
	  assert(ax->opt.min_auto == 0);
	  
	  if (ax->rescale_required)
		axis_scale(ax);
	  if (ax->scale.factor != 0.) {
		dist = ax->opt.obsrvd.max - (1. - SHIFT_PCT)*ax->opt.limits.max -
								SHIFT_PCT*ax->opt.limits.min;
		/* determine the amount of the shift */
		/* dist is now number of *units* to shift */
		dist *= ax->scale.factor; /* now number of tips to shift */
		dist /= QW_H_TPP; /* now number of pixels to shift */
		dist = ceil(dist) * QW_H_TPP; /* round up, back to tips */
		if (dist >= ax->window->width)
		  ax->window->redraw_required = 1;
		else {
		  shift = - (int) dist;
		  PictureCurrent(ax->window->pict_id);
		  if (ax->window->draw_direct)
			ShiftArea(0, 0, ax->window->height, ax->window->width, 0, shift);
		  else
			ShiftBy(QW_ALL, 0, shift);
		  Draw();
		}
		dist /= ax->scale.factor; /* number of *units* shifted */
		ax->opt.limits.min += dist;
		ax->opt.limits.max += dist;
		ax->scale.offset += dist;
	  }
	}
  } else {
	if (min_auto && ax->opt.obsrvd.min != ax->opt.limits.min) {
	  ax->opt.limits.min = ax->opt.obsrvd.min;
	  ax->rescale_required = 1;
	}
	if (max_auto && ax->opt.obsrvd.max != ax->opt.limits.max) {
	  ax->opt.limits.max = ax->opt.obsrvd.max;
	  ax->rescale_required = 1;
	}
  }
  ax->auto_scale_required = 0;
}

void axis_scale(RtgAxis *ax) {
  if (ax->opt.limits.min >= ax->opt.limits.max || ax->min_coord >= ax->max_coord) {
	/* set all scaling to zero */
	ax->scale.offset = 0.;
	ax->scale.factor = 0.;
	ax->scale.shift = 0;
  } else {
	int tpp;

	/* Weirdness: Using SetPointArea(y, x) you can display points
	   with y values in the range [QW_V_TPP, y]
	   and x values in the range  [0, x-2*QW_H_TPP]
	*/
	tpp = ax->is_y_axis ? 0 : QW_H_TPP;
	ax->scale.offset = ax->opt.limits.min;
	ax->scale.factor = (ax->max_coord - ax->min_coord - tpp) /
	  (ax->opt.limits.max - ax->opt.limits.min);
	ax->scale.shift = ax->is_y_axis ? QW_V_TPP : 0;
  }
  ax->rescale_required = 0;

  /* Here's a brute force action if I ever saw one: */
  ax->window->redraw_required = 1;
  /* eventually I could be more descriminating:
	  If I'm not drawing direct, I could erase only the graphs on
	  this axis. If drawing direct, I could limit my erasure to all
	  graphs in the overlay. But this should work....
  */
}

/* axis_draw() is supposed to draw the axis. For starters, I
   don't actually draw the axes, but I do reset the obsrvd
   fields on the axis in order to better support auto scaling
*/
void axis_draw(RtgAxis *ax) {
  RtgGraph *graph;
  RtgAxis *bx;

  ax->opt.obsrvd.min = 0;
  ax->opt.obsrvd.max = -1;
  for (graph = ax->window->graphs; graph != NULL; graph = graph->next) {
	bx = (ax->is_y_axis) ? graph->Y_Axis : graph->X_Axis;
	if (ax == bx) {
	  if (graph->lookahead != 0) {
		graph->lookahead->type->position_delete(graph->lookahead);
		graph->lookahead = NULL;
	  }
	  graph->looked_ahead = 0;
	}
  }
  ax->redraw_required = 0;
}

/* dastring_init() is useful for the most general initialization
   of dastring's. It is perfectly reasonable to use nl_strdup()
   instead if it is obvious that the new quantity is non-null,
   but that is not always the case...
*/
dastring dastring_init(const char *new) {
  if (new == 0 || *new == '\0') return NULL;
  else return nl_strdup(new);
}

/* dastring_update does a conditional free_memory(). Can be used
   with new==NULL to free memory before deleting the enclosing
   structure.
*/
void dastring_update(dastring *das, const char *new) {
  if (*das != 0) free_memory(*das);
  *das = dastring_init(new);
}

const char *dastring_value(dastring das) {
  return (das == 0) ? "" : das;
}

void axopts_init(RtgAxisOpts *to, RtgAxisOpts *from) {
  *to = *from;
  to->units = dastring_init(from->units);
}

void axopts_update(RtgAxisOpts *to, RtgAxisOpts *from) {
  dastring_update(&to->units, NULL);
  axopts_init(to, from);
}

const char *trim_spaces(const char *str) {
  static char *buf;
  static int buflen;
  int i, saw_space, space;
  
  for (saw_space = i = 0; str[i] != '\0'; i++) {
	if (str[i] == ' ') {
	  if (!saw_space) {
		space = i;
		saw_space = 1;
	  }
	} else saw_space = 0;
  }
  if (saw_space) {
	if (buflen < space+1) {
	  if (buflen != 0) free_memory(buf);
	  buflen = space+1;
	  buf = new_memory(buflen);
	}
	for (i = 0; i < space; i++)
	  buf[i] = str[i];
	buf[space] = '\0';
	return buf;
  } else return str;
}
