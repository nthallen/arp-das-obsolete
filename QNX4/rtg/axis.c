/* axis.c functions pertaining to axes */
#include <assert.h>
#include <windows/Qwindow.h>
#include "rtg.h"
#include "nortlib.h"

RtgAxisOpts *X_Axis_Opts, *Y_Axis_Opts;

RtgAxis *axis_create(BaseWin *bw, chandef *channel, int is_y_axis) {
  RtgAxis *axis, **ax;

  assert(bw != 0 && channel != 0 && channel->opts.Y.units != 0 &&
		channel->opts.X.units  != 0);
  axis = new_memory(sizeof(RtgAxis));
  
  /* New axes must go at the END of the list */
  for (ax = (is_y_axis ? &bw->y_axes : &bw->x_axes);
	   *ax != 0;
	   ax = &(*ax)->next) ;
  axis->next = NULL;
  *ax = axis;
  
  axis->window = bw; /* Is this necessary? */
  axis->auto_scale_required = 1;
  /* I won't bother initializing min_coord, max_coord or the scaling functions,
     I'll just indicate that a rescale is required and that the window must
	 be resized also, which will cause all the appropriate actions
  */
  axis->rescale_required = 1;
  bw->resize_required = 1;
  axis->redraw_required = 1;
  axis->is_y_axis = is_y_axis;
  if (is_y_axis) {
	if (Y_Axis_Opts != 0) axopts_init(&axis->opt, Y_Axis_Opts);
	else axopts_init(&axis->opt, &channel->opts.Y);
  } else {
	if (X_Axis_Opts != 0) axopts_init(&axis->opt, X_Axis_Opts);
	else axopts_init(&axis->opt, &channel->opts.X);
  }

  /* Force auto-limits if non specified */
  if (axis->opt.limits.min > axis->opt.limits.max)
	axis->opt.min_auto = axis->opt.max_auto = 1;

  return axis;
}

/* axis delete unlinks the axis from the window's list of axes and
   then deletes any graphs attached to it.
*/
void axis_delete(RtgAxis *ax) {
  RtgAxis **xp;
  BaseWin *bw;
  RtgGraph *graph;
  
  bw = ax->window;
  xp = (ax->is_y_axis ? &bw->y_axes : &bw->x_axes);
  for (; *xp != 0 && *xp != ax; xp = &(*xp)->next) ;
  assert(*xp != 0);
  *xp = ax->next;
  
  /* delete all graphs attached to this axis */
  for (;;) {
	for (graph = bw->graphs; graph != NULL; graph = graph->next) {
	  if (graph->X_Axis == ax || graph->Y_Axis == ax) break;
	}
	if (graph == NULL) break;
	graph_delete(graph);
  }

  bw->resize_required = 1;

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

  /* Extra code for scrolling x-axes required here!!! */
  if (min_auto && ax->opt.obsrvd.min != ax->opt.limits.min) {
	ax->opt.limits.min = ax->opt.obsrvd.min;
	ax->rescale_required = 1;
  }
  if (max_auto && ax->opt.obsrvd.max != ax->opt.limits.max) {
	ax->opt.limits.max = ax->opt.obsrvd.max;
	ax->rescale_required = 1;
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
  if (new == 0) return NULL;
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

void axopts_init(RtgAxisOpts *to, RtgAxisOpts *from) {
  *to = *from;
  to->units = dastring_init(from->units);
}

void axopts_update(RtgAxisOpts *to, RtgAxisOpts *from) {
  dastring_update(&to->units, NULL);
  axopts_init(to, from);
}
