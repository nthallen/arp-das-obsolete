/* axis.c functions pertaining to axes */
#include <assert.h>
#include "rtg.h"
#include "nortlib.h"

RtgAxisOpts *X_Axis_Opts, *Y_Axis_Opts;

RtgAxis *axis_create(BaseWin *bw, chandef *channel, int is_y_axis) {
  RtgAxis *axis, **ax;

  assert(bw != 0 && channel != 0 && channel->yunits != 0 &&
		channel->xunits  != 0);
  axis = new_memory(sizeof(RtgAxis));
  
  /* New axes must go at the END of the list */
  for (ax = (is_y_axis ? &bw->y_axes : &bw->x_axes); *ax != 0; ax = &(*ax)->next) ;
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
  axis->units = nl_strdup(is_y_axis ? channel->yunits : channel->xunits);
  if (is_y_axis) {
	axis->units = nl_strdup(channel->yunits);
	if (Y_Axis_Opts != 0) axis->opt = *Y_Axis_Opts;
	else axis->opt = channel->opts.Y;
  } else {
	axis->units = nl_strdup(channel->xunits);
	if (X_Axis_Opts != 0) axis->opt = *X_Axis_Opts;
	else axis->opt = channel->opts.X;
  }

  /* Force auto-limits if non specified */
  if (axis->opt.limits.min > axis->opt.limits.max)
	axis->opt.min_auto = axis->opt.max_auto = 1;

  #ifdef OPTIONS_WORKING
	/* Following are the public options */
	axis->opt.weight = 1;
	axis->opt.overlay = 0;
	axis->opt.limits.min = 0.;
	axis->opt.limits.max = -1.;
	axis->opt.obsrvd.min = 0;
	axis->opt.obsrvd.max = -1.;
	axis->opt.min_auto = 0;
	axis->opt.max_auto = 0;
	axis->opt.scope = 0;
	axis->opt.scroll = 0;
	axis->opt.normal = 0;
	axis->opt.single_sweep = 0;
	axis->opt.clear_on_trig = 0;
	/* labeling options { none for now } */
  #endif
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

  free_memory(ax->units);
  free_memory(ax);
}

/* Determines Min and Max values: does not scale the window */
void axis_auto_range(RtgAxis *ax) {
  BaseWin *bw;
  RtgGraph *graph;
  RtgAxis *bx;
  chandef *chan;
  RtgRange *range;
  int min_auto, max_auto;
  
  min_auto = ax->opt.min_auto;
  max_auto = ax->opt.max_auto;
  if (min_auto == 0 && max_auto == 0)
	return;
  bw = ax->window;
  for (graph = bw->graphs; graph != NULL; graph = graph->next) {
	bx = (ax->is_y_axis) ? graph->Y_Axis : graph->X_Axis;
	if (ax == bx) {
	  chan = graph->position->channel;
	  /* should look ahead to make sure the range is set?
	     This action should be channel-type specific */
	  range = (ax->is_y_axis) ? &chan->opts.Y.limits : &chan->opts.X.limits;
	  if (ax->opt.limits.min > ax->opt.limits.max) {
		ax->opt.limits.min = range->min;
		ax->opt.limits.max = range->max;
		ax->rescale_required = 1;
	  } else {
		if (min_auto && range->min < ax->opt.limits.min) {
		  ax->opt.limits.min = range->min;
		  ax->rescale_required = 1;
		}
		if (max_auto && range->max > ax->opt.limits.max) {
		  ax->opt.limits.max = range->max;
		  ax->rescale_required = 1;
		}
	  }
	}
  }
  if (min_auto && ax->opt.obsrvd.min < ax->opt.limits.min) {
	ax->opt.limits.min = ax->opt.obsrvd.min;
	ax->rescale_required = 1;
  }
  if (max_auto && ax->opt.obsrvd.max > ax->opt.limits.max) {
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
	ax->scale.offset = ax->opt.limits.min;
	ax->scale.factor =
	  (ax->max_coord - ax->min_coord) /
		(ax->opt.limits.max - ax->opt.limits.min);
	ax->scale.shift = 0;
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

void axis_draw(RtgAxis *ax) {
  ax->redraw_required = 0;
}
