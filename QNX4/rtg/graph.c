/* graph.c contains functions pertaining to graphs */
#include <windows/Qwindows.h>
#include <assert.h>
#include <string.h>
#include "rtg.h"
#include "nortlib.h"
#include "ssp.h"

/* graph_create(channel) Strategy:
   How will I get the creation options in there? Or are they changed after
   the fact? (overlay, weight)
   
   Search the window axes for a compatable Y axis (same units). If not
   found, stack a new one.
*/

void graph_create(const char *channel, char bw_ltr) {
  chandef *cc;
  BaseWin *bw;
  RtgAxis *x_ax, *y_ax;
  RtgGraph *graph;
  RtgAxisOpts *opts;
  
  cc = channel_props(channel); assert(cc != 0);
  bw = BaseWin_find(bw_ltr); assert(bw != 0);
  
  /* Locate or create the axes */
  x_ax = bw->x_axes;
  if (x_ax == 0)
	x_ax = axis_create(bw, cc, 0);
  
  /* we can use the same Y axis as the last one if:
	force_new == 0
	overlay != 0
	last axis has same units as new channel
  */
  if (Y_Axis_Opts != 0) opts = Y_Axis_Opts;
  else opts = &cc->opts.Y;
  y_ax = NULL;
  if (opts->force_new == 0 && opts->overlay != 0) {
	for (y_ax = bw->y_axes; y_ax != 0 && y_ax->next != 0; y_ax = y_ax->next);
	if (y_ax != 0 && strcmp(y_ax->opt.units, cc->opts.Y.units) != 0)
	  y_ax = NULL;
  }
  if (y_ax == 0)
	y_ax = axis_create(bw, cc, 1);

  /* Build the graph structure */
  graph = new_memory(sizeof(RtgGraph));
  graph->next = bw->graphs;
  bw->graphs = graph;
  graph->window = bw;
  graph->X_Axis = x_ax;
  graph->Y_Axis = y_ax;
  graph->position = position_create(cc);
  graph->lookahead = NULL;
  graph->looked_ahead = 0;
  graph->line_thickness = 1;
  graph->line_color = QW_RED;
  graph->line_style = 0;
  graph->symbol[0] = graph->symbol[1] = 0;
  graph->symbol_color = QW_BLUE;
  
  /* Add it to the graph ChanTree! */
  { RtgChanNode *CN;
	static int n_channels = 0;
	char gname[20];
	
	do {
	  sprintf(gname, "Graph%d", ++n_channels);
	  CN = ChanTree(CT_INSERT, CT_GRAPH, gname);
	} while (CN == 0);
	assert(CN != 0);
	graph->name = dastring_init(gname);
	CN->u.leaf.graph = graph;
  }
}

/* Unlinks the specified graph from the window's list.
   Must be careful
   of chicken and egg here, i.e. unlink the graph before touching
   axes and unlinking axes before touching graphs.
*/
void graph_delete(RtgGraph *graph) {
  BaseWin *bw;
  RtgGraph **gpp;
  RtgAxis *x_ax, *y_ax;

  ChanTree(CT_DELETE, CT_GRAPH, graph->name);
  x_ax = graph->X_Axis;
  y_ax = graph->Y_Axis;
  bw = graph->window;
  for (gpp = &bw->graphs; *gpp != 0 && *gpp != graph; gpp = &(*gpp)->next) ;
  assert(*gpp != 0);
  *gpp = graph->next;
  bw->redraw_required = 1;
  graph->position->type->position_delete(graph->position);
  if (graph->lookahead != 0)
	graph->position->type->position_delete(graph->lookahead);
  if (graph->name != 0)
	free_memory(graph->name);
  free_memory(graph);
  
  /* Now let's see if these axes have been orphaned */
  for (graph = bw->graphs; graph != NULL; graph = graph->next) {
	if (x_ax != 0 && graph->X_Axis == x_ax) x_ax = NULL;
	if (y_ax != 0 && graph->Y_Axis == y_ax) y_ax = NULL;
  }
  if (x_ax != 0) axis_delete(x_ax);
  if (y_ax != 0) axis_delete(y_ax);
}

/* graph_ndelete() deletes the named graph by calling graph_delete() */
void graph_ndelete(const char *name, char unrefd /*bw_ltr*/) {
  RtgChanNode *CN;
  
  CN = ChanTree(CT_FIND, CT_GRAPH, name);
  assert(CN != 0);
  graph_delete(CN->u.leaf.graph);
}

/* graph_nprops() deletes the named graph by calling graph_delete() */
void graph_nprops(const char *name, char unrefd /*bw_ltr*/) {
  Properties(name, GRAPH_PROPS);
}

/*
  Record observed extrema
  return non-zero if a rescale is required (number outside range and
	autoscaling on that extrema is enabled)
  otherwise set flags to one of the following:
    0 - point is within current limits
	1 - point is below current minimum
	2 - point is above current maximum
	3 - point is a non-number
  A non-number does not trigger rescaling.
  
  Modifying to use pair structure. limit_value will copy real value to
  clip value.
*/
static int limit_value(RtgAxis *ax, clip_coord *V) {
  if (!is_number(V->val)) {
	V->flag = 3;
	return 0;
  }
  V->clip = V->val;

  /* Automatically set minimum in scope or scroll modes */
  if ((!ax->is_y_axis) && ax->opt.min_auto &&
		(ax->opt.scope || ax->opt.scroll)) {
	double diff;
	diff = ax->opt.limits.max - ax->opt.limits.min;
	ax->opt.limits.min = ax->scale.offset = V->val;
	ax->opt.limits.max = V->val+diff;
	ax->opt.min_auto = 0;
  }

  if (ax->opt.obsrvd.min > ax->opt.obsrvd.max)
	ax->opt.obsrvd.min = ax->opt.obsrvd.max = V->val;
  else if (V->val < ax->opt.obsrvd.min)
	ax->opt.obsrvd.min = V->val;
  else if (V->val > ax->opt.obsrvd.max)
	ax->opt.obsrvd.max = V->val;
  V->flag = 0;
  if (V->val < ax->opt.limits.min) {
	V->flag |= 1;
	if (ax->opt.min_auto) {
	  ax->auto_scale_required = 1;
	  return 1;
	}
  } else if (V->val > ax->opt.limits.max) {
	V->flag |= 2;
	if (ax->opt.max_auto) {
	  ax->auto_scale_required = 1;
	  return 1;
	}
  }
  return 0;
}

/* scale_value assumes val is within range, since clipping has already
   taken place
*/
static void scale_value(RtgAxis *ax, clip_coord *V) {
  long int cv;
  
  cv = (long int) ((V->clip - ax->scale.offset) * ax->scale.factor);
  cv += ax->scale.shift;
  V->coord = (unsigned short) cv;
}

#define N_POINTS 50
static QW_CXY_COORD xy[N_POINTS];
static int n_xy_pts = 0;

static void buffer_xy(unsigned short X, unsigned short Y) {
  assert(n_xy_pts < N_POINTS);
  xy[n_xy_pts].x = X;
  xy[n_xy_pts].y = Y;
  n_xy_pts++;
}

/* scale_pair, like scale_value, assumes val is within range,
   since clipping has already taken place.
*/
static void scale_pair(RtgGraph *graph, clip_pair *P) {
  scale_value(graph->X_Axis, &P->X);
  scale_value(graph->Y_Axis, &P->Y);
  buffer_xy(P->X.coord, P->Y.coord);
}

/* Flushes any points currently in the xy buffer
   If symbol is non-zero, it is used to draw a
   symbol at each point. This will have to be expanded to
   use line_thickness, line_color, etc. but I'll need the
   manual to do that.
*/
static void flush_points(RtgGraph *graph, char *symbol) {
  assert(graph != NULL && graph->window != NULL);
  if (n_xy_pts > 0) {
	int line_color;
	char dp_opts[6], *dpp;
	char *sym;

	PictureCurrent(graph->window->pict_id);
	SetLineThickness("n", graph->line_thickness*QW_V_TPP);
	SetColor("n", graph->symbol_color);
	line_color = graph->line_thickness ? graph->line_color : 0;
	DrawAt(graph->Y_Axis->min_coord, graph->X_Axis->min_coord);
	SetPointArea(graph->Y_Axis->n_coords,
		graph->X_Axis->n_coords);
	dpp = dp_opts;
	if (graph->window->draw_direct) *dpp++ = '!';
	*dpp++ = ';'; *dpp++ = 'K';
	switch (*symbol) {
	  case '.': sym = NULL; *dpp++ = 'D'; break;
	  case '*': sym = NULL; break;
	  case 0:
	  case ' ': sym = NULL; *dpp++ = 'N'; break;
	  default:
		sym = symbol; *dpp++ = 'C'; break;
	}
	*dpp = '\0';
	DrawPoints(n_xy_pts, (QW_XY_COORD *)xy, sym, line_color,
	  dp_opts, NULL);
	Draw();
  }
  n_xy_pts = 0;
}

/* Return non-zero if points already drawn should be flushed.
   Backs up the position by one just in case...
*/
static int flush_anyway(RtgGraph *graph, clip_pair *P) {
  /* We're out of bounds and auto-ranging.
	 I will backup one just in case we're coming back without a redraw
  */
  graph->position->type->position_move(graph->position, -1L);

  /* Flush current points if:
		Xmax was exceeded &&
		((scroll && !(clear_on_trig && scope && auto)) ||
		  (scope && auto && !clear_on_trig))
		[auto === !normal]
	 In any case, don't process more points until
	 after returning for auto range processing
  */
  if (P->X.flag & 2) {
	RtgAxisOpts *xo;

	xo = &graph->X_Axis->opt;
	if ((xo->scroll &&
		  ((!xo->clear_on_trig) || (!xo->scope) || xo->normal)) ||
		(xo->scope && (!xo->normal) && (!xo->clear_on_trig))) {
	  return 1;
	}
  }
  return 0;
}

static void plot_symbols(RtgGraph *graph) {
  chanpos *pos;
  chantype *type;
  clip_pair P;

  pos = graph->position;
  type = pos->type;
  if (graph->symbol[0] == 0) {
	/* nothing to do: advance to eof */
	type->position_move(pos, LONG_MAX);
	return;
  }
  n_xy_pts = 0;
  while (n_xy_pts < N_POINTS) {
	if (type->position_data(pos, &P.X.val, &P.Y.val) == 0)
	  break;
	if (limit_value(graph->X_Axis, &P.X) ||
		limit_value(graph->Y_Axis, &P.Y)) {
	  if (flush_anyway(graph, &P))
		break;
	  return;
	}
	if (P.X.flag == 0 && P.Y.flag == 0)
	  scale_pair(graph, &P);
  }
  flush_points(graph, graph->symbol);
}

void lookahead(RtgGraph *graph) {
  clip_coord X, Y;
  chantype *type;
  int i;

  assert(graph->looked_ahead == 0);

  type = graph->position->type;
  if (graph->lookahead == 0) {
	graph->lookahead = position_duplicate(graph->position);
	assert(graph->lookahead != 0);
  }

  for (i = 0; i < 1000; i++) {
	if (type->position_data(graph->lookahead, &X.val, &Y.val) == 0) {
	  type->position_delete(graph->lookahead);
	  graph->lookahead = NULL;
	  graph->looked_ahead = 1;
	  return;
	}
	limit_value(graph->X_Axis, &X);
	limit_value(graph->Y_Axis, &Y);
  }
}

void plot_graph(RtgGraph *graph) {
  chanpos *pos;
  chantype *type;
  clip_pair pairs[2], *P1, *P2;
  int pair_no, clipped;

  if (graph->line_thickness == 0 || graph->line_color == 0) {
	plot_symbols(graph);
	return;
  }
  pos = graph->position;
  type = pos->type;
  type->position_move(pos, -1L);
  n_xy_pts = 0;
  pair_no = 0;
  P1 = pairs+pair_no;
  
  /* We need to get one point to start: we throw out
     non-numbers here, since they aren't a good first
	 point
  */
  for (;;) {
	if (type->position_data(pos, &P1->X.val, &P1->Y.val) == 0 ||
		limit_value(graph->X_Axis, &P1->X) ||
		limit_value(graph->Y_Axis, &P1->Y))
	  return;
	if (P1->Y.flag != 3 && P1->X.flag != 3) break;
  }
  if (P1->X.flag == 0 && P1->Y.flag == 0)
	scale_pair(graph, P1);

  for ( ; n_xy_pts < N_POINTS; P1 = P2 ) {
	pair_no = 1 - pair_no;
	P2 = pairs+pair_no;
	if (type->position_data(pos, &P2->X.val, &P2->Y.val) == 0)
	  break;
	if (limit_value(graph->X_Axis, &P2->X) ||
		limit_value(graph->Y_Axis, &P2->Y)) {
	  if (flush_anyway(graph, P2))
		break;
	  return;
	}

	/* Handle the non-number case: */
	if (P2->Y.flag == 3 || P2->X.flag == 3)
	  break;

	clipped = clip_line(graph, P1, P2);
	if (clipped != 4) {
	  if (clipped & 1) { /* first value clipped? */
		scale_pair(graph, P1);
	  } else if (graph->symbol[0] && (clipped & 2)) {
		flush_points(graph, graph->symbol);
		/* then buffer first point */
		buffer_xy(P1->X.coord, P1->Y.coord);
	  }
	  /* scale and buffer second point */
	  scale_pair(graph, P2);
	  if ((clipped & 2) || (graph->symbol && (clipped & 1))) {
		flush_points(graph, ""); /* flush w/o symbols */
		if (clipped & 2)
		  return;
		/* rebuffer second point to plot with symbols */
		buffer_xy(P2->X.coord, P2->Y.coord);
	  }
	}
  }
  flush_points(graph, graph->symbol);
}
