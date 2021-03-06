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
 * A BaseWin is unused if it doesn't have its pict_id is 0
 *
 * $Log$
 * Revision 1.15  2007/03/28 18:51:31  nort
 * Migrated from RCS
 *
 * Revision 1.14  1998/06/25 02:55:05  nort
 * Outstanding changes
 *
 * Revision 1.13  1995/12/19  21:40:01  nort
 * Support for Icon labels, Exposed notification
 *
 * Revision 1.12  1995/12/19  19:36:51  nort
 * *** empty log message ***
 *
 * Revision 1.11  1995/02/14  21:04:50  nort
 * Scripting is Working
 *
 * Revision 1.10  1995/02/14  15:16:02  nort
 * Halfway through scripting
 *
 * Revision 1.5  1994/12/13  16:10:16  nort
 * Realtime!
 *
 * Revision 1.4  1994/12/08  17:25:34  nort
 * Working on limits problems
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

/* This menu currently supports:
   [gcw][PCD]
   Where the first letter stands for Graph, Channel or Window
   and the second stands for Properties, Create or Delete
   
   also
   C[LS] for Configuration Load|Save
*/
static char pane_menu[] =
  /* Graph Props (grf defd) Create (chan defd) Delete (grf defd) */
  "Graph^R@(<Graph>Properties...%s|gP;Create%s|gC;Delete%s|gD);"

  /* Channel Props (chan defd) Create Delete (chan defd) */
  "Channel^R@(<Channel>Properties...%s|cP;"
    "Create^R@(<Create>Spreadsheet|cS;Test|cC);Delete%s|cD);"

  /* Window Props Create Delete */
  "Window^R@(<Window>Properties...|wP;Create|wC;Delete|wD);"
  ";-;Config^R@(<Config>Load|CL;Save|CS)";

static int win_handler(QW_EVENT_MSG *msg, char *unrefd /* label */) {
  BaseWin *bw;
  char *buf, *chan_opt, *graph_opt;
  
  unrefd = unrefd; /* just to quiet the compiler */
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
        graph_opt = ChanTree_defined(CT_GRAPH) ? "" : "^~";
        sprintf(buf, pane_menu, graph_opt, chan_opt, graph_opt,
            chan_opt, chan_opt);
        Menu( bw->bw_label, NULL, 1, buf, "A");
        free_memory(buf);
        return 1;
      }
      break;
    case QW_PROPERTIES:
      Properties( "", "RP", 1 );
      return 1;
    case QW_EXPOSED:
      if ( bw->draw_direct )
        bw->redraw_required = 1;
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

static void window_nprops(const char *name, char unrefd /*bw_ltr*/) {
  /* Properties(name, WINDOW_PROPS); */
  unrefd = unrefd; /* to quiet the compiler */
  Properties(name, "WP", 1);
}

/* The basewindow key_handler handles responses from the pane menu.
   The label is the BaseWin label, so label[1] is the bw_ltr.
   The BaseWin itself can be extracted via BaseWin_find(bw_ltr);
*/
static int key_handler(QW_EVENT_MSG *msg, char *label) {
  assert(label != NULL);
  switch (msg->hdr.key[0]) {
    case 'c': /* Channel */
      channel_opts(msg->hdr.key[1], label[1]);
      return 1;
    case 'g': /* Graph */
      switch (msg->hdr.key[1]) {
        case 'P': /* Properties */
          if (ChanTree_defined(CT_GRAPH))
            ChanTree_Menu(CT_GRAPH, "Properties", graph_nprops, label[1]);
          break;
        case 'C': /* Create */
          if (channels_defined())
            ChanTree_Menu(CT_CHANNEL, "Select Channel", graph_create, label[1]);
          else nl_error(1, "No Channels Defined");
          break;
        case 'D': /* Delete */
          if (ChanTree_defined(CT_GRAPH))
            ChanTree_Menu(CT_GRAPH, "Delete Graph", graph_ndelete, label[1]);
          else nl_error(1, "No Graphs Defined");
          break;
        default:
          return 0;
      }
      return 1;
    case 'w': /* Window */
      switch (msg->hdr.key[1]) {
        case 'P': /* Properties */
          if (ChanTree_defined(CT_WINDOW))
            ChanTree_Menu(CT_WINDOW, "Properties", window_nprops, label[1]);
          break;
        case 'C': /* Create */
          New_Base_Window( NULL );
          break;
        case 'D': /* Delete */
          Del_Base_Window(label[1]);
          break;
        default:
          return 0;
      }
      return 1;
    case 'C':
      switch (msg->hdr.key[1]) {
        case 'L': /* Load configuration */
          script_load( GlobOpts.config_file );
          break;
        case 'S':
          script_create( GlobOpts.config_file );
          break;
      }
      return 1;
    case 'N':
      return 1;
    default:
      break;
  }
  return 0;
}

static void basewin_open(BaseWin *bw) {
  assert(bw != 0);
  assert(bw->wind_id == 0);

  if (bw->pict_id == 0) return;
  PictureCurrent(bw->pict_id);
  
  /* WmWindowPropRead(bw->bw_name, &wnd); */
  if (bw->row >= 0)
    WindowAt(bw->row, bw->col, NULL, NULL);

  { char *wind_opts, *wopt_fmt;
    wopt_fmt = "%s%s;MNs:%s/rtgicon.pict";
    wind_opts = new_memory(strlen(wopt_fmt) + load_path_len);
    sprintf(wind_opts, wopt_fmt,
                        bw->title_bar ? "" : "N",
                        bw->fix_front ? "f" : "",
                        load_path);
    bw->wind_id = WindowOpen(bw->bw_name, bw->height, bw->width,
      /* options */ wind_opts,  /* actions */ "RX",
      bw->title, bw->pict_id);
    /* Icon label could go in wind_opts, but this seems easier */
    WindowIcon( NULL, bw->title, QW_KEEP, QW_KEEP, 
                QW_TRANSPARENT );
    free_memory(wind_opts);
  }

  /* This may not always be correct! */
  PaneView(0, bw->width, -1, -1, -1, -1, -1);
  set_win_handler(bw->wind_id, win_handler);
  WmWindowPropEnable(bw->bw_name);
}

void basewin_close(BaseWin *bw) {
  assert(bw != 0);
  if (bw->wind_id != 0) {
    WindowCurrent(bw->wind_id);
    WindowClose();
    del_win_handler(bw->wind_id);
    bw->wind_id = 0;
  }
}

/* returns non-zero on error */
int New_Base_Window( const char *name ) {
  BaseWin *bw;

  for (bw = BaseWins;
       bw != NULL && ( bw->wind_id != 0 || bw->pict_id != 0 );
       bw = bw->next);
  if (bw == NULL) {
    if (n_basewins == 0)
      set_key_handler('B', key_handler);
    else if (n_basewins >= 26) {
      nl_error(2, "Maximum number of windows reached");
      return 1;
    }
    bw = new_memory(sizeof(BaseWin));
    bw->bw_id = n_basewins++;
    assert(bw->bw_id < 26);
    bw->bw_label[0] = 'B';
    bw->bw_label[1] = 'A' + bw->bw_id;
    bw->bw_label[2] = '\0';
    sprintf(bw->bw_name, "RTG_%d", bw->bw_id);
    bw->next = BaseWins;
    BaseWins = bw;
  }
  bw->wind_id = bw->pict_id = 0;
  bw->row = bw->col = -1;
  bw->height = 3000;
  bw->width = 4000;
  bw->graphs = NULL;
  bw->x_axes = NULL;
  bw->y_axes = NULL;
  bw->triggers = NULL;
  bw->resize_required = 0;
  bw->redraw_required = 0;
  bw->draw_direct = 0;
  bw->title = NULL;
  bw->bkgd_color = QW_WHITE;
  bw->bkgd_pattern = QW_SOLID_PAT;
  bw->title_bar = 1;
  bw->fix_front = 0;

  { RtgChanNode *CN;

    if ( name != 0 ) {
      CN = ChanTree(CT_INSERT, CT_WINDOW, name);
      if ( CN == 0 ) {
        nl_error(2, "Window %s already exists", name );
        return 1;
      }
      bw->title = dastring_init( name );
    } else {
      bw->title = dastring_init(ChanTreeWild(CT_WINDOW, "RTG%d"));
      CN = ChanTree(CT_FIND, CT_WINDOW, bw->title);
    }
    assert(CN != 0 && CN->u.leaf.bw == 0);
    CN->u.leaf.bw = bw;
  }
  /* bw->pict_id = Picture(bw->bw_name, NULL); */
  bw->pict_id = PictureOpen( bw->bw_name, NULL, NULL, bw->bkgd_color,
          bw->bkgd_pattern, NULL, NULL );
  assert( bw->pict_id != 0 );
  n_winsopen++;
  return 0;
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
  PropCancel( bw->title, "WP", "P" );
  { RtgChanNode *CN;
    CN = ChanTree(CT_FIND, CT_WINDOW, bw->title);
    assert(CN != 0);
    ChanTree(CT_DELETE, CT_WINDOW, bw->title);
  }
  basewin_close(bw);
  PictureClose(bw->pict_id);
  bw->pict_id = 0;
  if (--n_winsopen == 0) exit(0);

  /* Must delete each axis (and each graph) */
  while (bw->x_axes != 0) axis_delete(bw->x_axes);
  while (bw->y_axes != 0) axis_delete(bw->y_axes);
  assert(bw->graphs == 0);
}

/* Basewin_record() records the current position of the specified BaseWin.
   This is done in windprop when the window must be closed and reopened
   at the same location, and in rtgscript before the window properties
   are written out.
*/
void Basewin_record( BaseWin *bw ) {
  QW_RECT_AREA warea;
    
  if (bw->wind_id != 0) {
    WindowCurrent(bw->wind_id);
    WindowInfo(NULL, &warea, NULL, NULL, NULL, NULL);
    bw->row = warea.row;
    bw->col = warea.col;
  }
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

/* basewin_erase() clears the current window of everything and marks it
   for redraw. Side effect is that window and picture are current.
*/
void basewin_erase( BaseWin *bw ) {
  WindowCurrent(bw->wind_id);
  PictureCurrent(bw->pict_id);

  if (bw->draw_direct) {
    SetFill("r", bw->bkgd_color, bw->bkgd_pattern);
    DrawAt(0, 0);
    DrawRect(bw->height, 2*bw->width, "!", NULL);
  } else Erase((char const __far *)QW_ALL);
  
  bw->redraw_required = 1;
}

/* This routine clears the window and marks each axis and graph for redraw */
static void redraw_basewin(BaseWin *bw) {
  RtgAxis *ax;
  RtgGraph *graph;

  basewin_erase( bw );

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

/* scale_axes() returns 1 while there is still work to do */
static int scale_axes(RtgAxis *ax) {
  for ( ; ax != NULL; ax = ax->next) {
    if (ax->auto_scale_required) {
      axis_auto_range(ax);
      return 1;
    }
    if (ax->rescale_required) {
      axis_scale(ax);
      return 1;
    }
  }
  return 0;
}

/* draw_axes() returns 1 while there is still work to do */
static int draw_axes(RtgAxis *ax) {
  for ( ; ax != NULL; ax = ax->next) {
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

  if (bw->pict_id == 0)
    return 0;
  if (bw->wind_id == 0)
    basewin_open(bw);
  if (bw->wind_id == 0 || bw->pict_id == 0) return 0;
  if (bw->resize_required) {
    resize_basewin(bw);
    return 1;
  }

  if ( scale_axes( bw->x_axes ) || scale_axes( bw->y_axes ) )
    return 1;

  for (graph = bw->graphs; graph != NULL; graph = graph->next) {
    if (graph->position->reset != 0)
      bw->redraw_required = 1;
  }

  if (bw->redraw_required) {
    redraw_basewin(bw);
    return 1;
  }

  if (draw_axes(bw->x_axes) || draw_axes(bw->y_axes) )
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
static int plotting_enabled = 1;
void plotting_enable( int enable ) {
  plotting_enabled = enable;
}

int plotting(void) {
  BaseWin *bw;
  if ( plotting_enabled ) {
    for (bw = BaseWins; bw != NULL; bw = bw->next)
      if (plot_window(bw)) return 1;
  }
  return 0;
}
