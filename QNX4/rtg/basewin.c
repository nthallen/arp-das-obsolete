/* basewin.c Defines BaseWindow objects
 * For handlers, I will use the label letter 'B'
 *  Second letter will indicate local action. For now I will use:
 *  P  Properties
 *  G  Create Graph
 *  g  Delete Graph
 *  W  Create Window
 *  w  Delete Window
 *  C  Clear
 *  T  Trigger
 *  A  Arm
 *  c  Channels (Create|cC, Delete|cD, Props|cP)
 *  p  Graphs
 *  N  No Op (for testing)
 *
 * I will limit the number of Base Windows to 26 (A-Z) and take
 * advantage of this limitation to get 1-letter identifiers as to
 * which window is referenced. This is a relatively arbitrary
 * limitation, but I think it is amply realistic. However, I must
 * come up with some scheme for reusing the numbers... How about
 * when a window is deleted, don't remove it from the chain, just
 * zero out the pict_id and wind_id and delete the window handler.
 * Then when a new one is requested, check first on the chain for
 * a currently unused one.
 *
 * $Log$
 * Revision 1.4  1994/12/08  17:25:34  nort
 * Working on limits problems
 *
 * Revision 1.3  1994/12/07  16:32:19  nort
 * *** empty log message ***
 *
 * Revision 1.2  1994/11/01  21:50:56  nort
 * *** empty log message ***
 *
 * Revision 1.1  1994/10/31  18:49:25  nort
 * Initial revision
 *
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows/Qwindows.h>
#include "rtg.h"
#include "nortlib.h"
#pragma off (unreferenced)
  static char
	rcsid[] = "$Id$";
#pragma on (unreferenced)

static int n_basewins=0, n_winsopen=0;
/* static QW_WNDPROP wnd; */

BaseWin *BaseWins = NULL;

static char pane_menu[] =
  "Properties^~|P"
  ";-;Create^R@(<Create>Graph%s|G;Window|W);"
  "Delete^R@(<Delete>Graph^~|g;Window|w)"
  ";-;Clear^~|C;Trigger^~|T;Arm^~|A"
  ";-;Channels^R@(<Channels>Properties...%s|cP;Create|cC;Delete%s|cD;Spreadsheet|cS)"
  ";Graphs|p";


static int win_handler(QW_EVENT_MSG *msg, char *label) {
  BaseWin *bw;
  char *buf, *chan_opt;
  
  /* First, identify the BaseWin */
  for (bw = BaseWins; bw != NULL && bw->wind_id != msg->hdr.window;
		  bw = bw->next);
  assert(bw != NULL);

  switch (msg->hdr.action) {
	case QW_CLICK:
	  if (msg->hdr.code == 'M') {
		/* This menu might need to be customized as follows:
			Channels/{Delete|Properties} should be dimmed if
			no channels are defined
		*/
		buf = new_memory(strlen(pane_menu));
		chan_opt = channels_defined() ? "" : "^~";
		sprintf(buf, pane_menu, chan_opt, chan_opt, chan_opt);
		Menu( bw->bw_label, "Pane", 1, buf, "A");
		free_memory(buf);
		return 1;
	  }
	  break;
	case QW_PROPERTIES:
	  Tell("BW Handler Props", bw->bw_label);
	  return 1;
	case QW_RESIZED:
	  if (msg->hdr.code != 'I')
		bw->resize_required = 1;
	  return 1;
	default:
	  break;
  }
  return 0;
}

/* The basewindow key_handler handles responses from the pane menu.
   The label is the BaseWin label, so label[1] is the bw_ltr.
   The BaseWin itself can be extracted via BaseWin_find(bw_ltr);
*/
static int key_handler(QW_EVENT_MSG *msg, char *label) {
  BaseWin *bw;
  void graph_select(RtgGraph *graphs); /* move to rtg.h sometime */

  assert(label != NULL);
  switch (msg->hdr.key[0]) {
	case 'P':
	  Tell("BW Key Props", label);
	  return 1;
	case 'W':
	  New_Base_Window();
	  return 1;
	case 'w':
	  Del_Base_Window(label[1]);
	  return 1;
	case 'c':
	  channel_opts(msg->hdr.key[1], label[1]);
	  return 1;
	case 'G':
	  if (channels_defined())
		channel_menu("Select Channel", graph_create, label[1]);
	  else Tell("Create Graph", "No Channels Defined");
	  return 1;
	case 'N':
	  return 1;
	case 'p':
	  bw = BaseWin_find(label[1]);
	  if (bw != 0)
		graph_select(bw->graphs);
	  return 1;
	default:
	  break;
  }
  return 0;
}

void New_Base_Window(void) {
  char wind_name[10];
  BaseWin *bw;

  for (bw = BaseWins;
	   bw != NULL && bw->wind_id != 0 && bw->pict_id != 0;
	   bw = bw->next);
  if (bw == NULL) {
	if (n_basewins == 0)
	  set_key_handler('B', key_handler);
	bw = new_memory(sizeof(BaseWin));
	bw->bw_id = n_basewins++;
	assert(bw->bw_id < 26);
	bw->bw_label[0] = 'B';
	bw->bw_label[1] = 'A' + bw->bw_id;
	bw->bw_label[2] = '\0';
	bw->wind_id = bw->pict_id = 0;
	bw->next = BaseWins;
	BaseWins = bw;
	bw->width = bw->height = 0;
	bw->graphs = NULL;
	bw->x_axes = NULL;
	bw->y_axes = NULL;
	bw->triggers = NULL;
	bw->resize_required = 0;
	bw->redraw_required = 0;
	bw->draw_direct = 0;
	bw->title = NULL;
	bw->bkgd_color = QW_WHITE;
  }

  if (bw->bw_id == 0) strcpy(wind_name, "RTG");
  else sprintf(wind_name, "RTG%d", bw->bw_id);
  bw->title = nl_strdup(wind_name);
  /* WmWindowPropRead(wind_name, &wnd); */

  bw->pict_id = Picture(wind_name, NULL);
  bw->wind_id = WindowOpen(wind_name, 3000, 4000,
    /* options */ "D;MNs:" SRCDIR "rtgicon.pict",
    /* actions */ "R",
	wind_name, bw->pict_id);
  n_winsopen++;
  set_win_handler(bw->wind_id, win_handler);
  WmWindowPropEnable(wind_name);
}

BaseWin *BaseWin_find(char bw_ltr) {
  BaseWin *bw;
  int bw_id;
  
  bw_id = bw_ltr - 'A';
  for (bw = BaseWins;
	   bw != NULL && bw->bw_id != bw_id;
	   bw = bw->next);
  if (bw != NULL && (bw->wind_id == 0 || bw->pict_id == 0))
	bw = NULL;
  return bw;
}

/* I'm not freeing the structure so I can keep track of holes for later
   creation.
 */
static void Del_Base_Window(char bw_ltr) {
  BaseWin *bw;

  bw = BaseWin_find(bw_ltr);
  assert(bw != NULL);
  WindowCurrent(bw->wind_id);
  WindowClose();
  del_win_handler(bw->wind_id);
  bw->wind_id = bw->pict_id = 0;
  if (--n_winsopen == 0) exit(0);

  /* Must delete each axis (and each graph) */
  while (bw->x_axes != 0) axis_delete(bw->x_axes);
  while (bw->y_axes != 0) axis_delete(bw->y_axes);
  assert(bw->graphs == 0);
}

/* resize_basewin determines the sizes of all the axes and marks them for
   rescale if necessary.
   Sum all the weights
*/
static void resize_basewin(BaseWin *bw) {
  unsigned short total_weight, weight;
  unsigned short min_coord, max_coord, height;
  RtgAxis *ax, *bx;
  int picture;
  QW_RECT_AREA region, view;

  bw->resize_required = 0;

  WindowCurrent(bw->wind_id);
  PaneInfo(&region, &view, &picture);
  assert(picture == bw->pict_id);
  bw->width = view.width;
  bw->height = view.height;
  
  total_weight = 0;
  for (ax = bw->y_axes; ax != 0; ) {
	weight = ax->opt.weight;
	for (bx = ax->next; bx != 0 && bx->opt.overlay != 0; bx = bx->next) {
	  if (bx->opt.weight > weight)
		weight = bx->opt.weight;
	}
	total_weight += weight;
	/* propagate the region's weight to all axes */
	for (; ax != 0 && ax != bx; ax = ax->next)
	  ax->opt.weight = weight;
  }
  if (total_weight == 0) return;
  min_coord = 0;
  for (ax = bw->y_axes; ax != 0; ) {
	weight = ax->opt.weight;
	height = (view.height * weight)/(total_weight * QW_V_TPP);
	height *= QW_V_TPP;
	max_coord = min_coord + height - QW_V_TPP;
	for (;;) {
	  if (ax->min_coord != min_coord ||
		  ax->max_coord != max_coord) {
		ax->min_coord = min_coord;
		ax->max_coord = max_coord;
		ax->n_coords = height;
		ax->rescale_required = 1;
	  }
	  ax = ax->next;
	  if (ax == 0 || ax->opt.overlay == 0) break;
	}
	total_weight -= weight;
	view.height -= height;
	min_coord += height;
  }
  
  /* Now the X Axis */
  for (ax = bw->x_axes; ax != NULL; ax = ax->next) {
	if (ax->min_coord != view.width || ax->n_coords != view.width) {
	  ax->min_coord = view.width;
	  ax->n_coords = view.width;
	  ax->max_coord = ax->min_coord + ax->n_coords - QW_H_TPP;
	  ax->rescale_required = 1;
	}
  }
}

/* This routine clears the window and marks each axis and graph for redraw */
static void redraw_basewin(BaseWin *bw) {
  RtgAxis *ax;
  RtgGraph *graph;

  WindowCurrent(bw->wind_id);
  PictureCurrent(bw->pict_id);
  if (bw->draw_direct) {
	SetFill("r", bw->bkgd_color, QW_SOLID_PAT);
	DrawAt(0, 0);
	DrawRect(bw->height, 2*bw->width, "!", NULL);
  } else Erase((char const __far *)QW_ALL);
  
  /* Move out one pane-width (to support scrolling) */
  Draw();
  ax = bw->x_axes;
  if (ax != 0) {
	QW_BOX size;

	size._height = size._width = 0;
	size.height = bw->height;
	size.width = ax->max_coord;
	PictureChange(QW_KEEP, 0, -1, &size, QW_KEEP);
	PaneCurrent(0);
	PaneView(0, ax->min_coord, bw->pict_id, -1, -1, -1, -1);
  }
  
  for (ax = bw->x_axes; ax != 0; ax = ax->next) ax->redraw_required = 1;
  for (ax = bw->y_axes; ax != 0; ax = ax->next) ax->redraw_required = 1;
  for (graph = bw->graphs; graph != NULL; graph = graph->next)
	graph->position->type->position_rewind(graph->position);

  bw->redraw_required = 0;
}

/* plot_axes() returns 1 while there is still work to do */
static int plot_axes(RtgAxis *ax) {
  for ( ; ax != NULL; ax = ax->next) {
	if (ax->auto_scale_required) {
	  axis_auto_range(ax);
	  return 1;
	}
	if (ax->rescale_required) {
	  axis_scale(ax);
	  return 1;
	}
	if (ax->redraw_required) {
	  axis_draw(ax);
	  return 1;
	}
  }
  return 0;
}

/* plot_window() returns 1 while there is still work to be done */
static int plot_window(BaseWin *bw) {
  RtgGraph *graph;

  if (bw->wind_id == 0 || bw->pict_id == 0) return 0;
  if (bw->resize_required) {
	resize_basewin(bw);
	return 1;
  } else if (bw->redraw_required) {
	redraw_basewin(bw);
	return 1;
  }

  /* This has to be refigured to deal with "regions" better */
  if (plot_axes(bw->x_axes))
	return 1;
  if (plot_axes(bw->y_axes))
	return 1;

  for (graph = bw->graphs; graph != NULL; graph = graph->next) {
	if (graph->position->at_eof == 0) {
	  plot_graph(graph);
	  return 1;
	}
  }
  return 0;
}

/* plotting() is called from the main receive loop. It is intended to perform
   some amount of plotting work and then return. If there is no work to be
   done, it should return 0. The reason this doesn't just plot everything,
   is that we want to reply to messages from either the windowing system
   or the data stream as quickly as possible.
   
   I will implement this function hierarchically, checking each of the
   BaseWins, and having each of them check each of their axes and then
   each of their graphs for work to be done.
*/
int plotting(void) {
  BaseWin *bw;
  
  for (bw = BaseWins; bw != NULL; bw = bw->next)
	if (plot_window(bw)) return 1;
  return 0;
}
