/* rtg.h definitions for rtg
 * $Log$
 * Revision 1.4  1994/12/07  16:32:09  nort
 * *** empty log message ***
 *
 * Revision 1.3  1994/11/01  21:50:43  nort
 * Restructuring of Axis Options
 *
 * Revision 1.2  1994/10/31  18:49:17  nort
 * *** empty log message ***
 *
 */

#define SRCDIR "/usr/local/src/das/rtg/"

typedef const char *dastring;

typedef struct rtg_chanpos {
  struct rtg_chanpos *next;
  struct rtg_chandef *channel;
  struct rtg_chantype *type; /* not redundant! */
  int position_id;
  unsigned char at_eof:1;
  unsigned char expired:1;
  unsigned char reset:1;
  unsigned char deleted:1;
} chanpos;

typedef struct {
  double min, max;
} RtgRange;

typedef struct {
  dastring units;
  RtgRange limits;
  RtgRange obsrvd;
  unsigned short weight;
  unsigned char overlay:1;
  unsigned char force_new:1;
  unsigned char min_auto:1;
  unsigned char max_auto:1;
  unsigned char scope:1;
  unsigned char scroll:1;
  unsigned char normal:1;
  unsigned char single_sweep:1;
  unsigned char clear_on_trig:1;
  /* labeling options { none for now } */
} RtgAxisOpts;

typedef struct {
  RtgAxisOpts X;
  RtgAxisOpts Y;
} RtgAxesOpts;

typedef struct rtg_chandef {
  dastring name;
  struct rtg_chantype *type;
  chanpos *positions;
  int channel_id;
  RtgAxesOpts opts;
} chandef;

typedef struct rtg_chantype {
  void (* channel_delete)(chandef *);
  int (* position_create)(chandef *);
  int (* position_duplicate)(chanpos *);
  void (* position_delete)(chanpos *);
  int (* position_rewind)(chanpos *);
  int (* position_data)(chanpos *, double *X, double *Y);
  int (* position_move)(chanpos *, long int index);
  RtgAxesOpts ResetOpts, DfltOpts;
} chantype;

typedef struct bwstr {
  struct bwstr *next;
  int wind_id;
  int pict_id;
  int bw_id; /* unique ID number */
  char bw_label[3]; /* 'A' + bw_id */
  unsigned short width, height; /* Current width,height of Pane */
  struct rtg_grph *graphs;
  struct rtg_axis *x_axes;
  struct rtg_axis *y_axes;
  struct rtg_trig *triggers;
  unsigned char resize_required:1;
  unsigned char redraw_required:1;
  unsigned char draw_direct:1;

  /* Following are the public options */
  dastring title;
  int bkgd_color;
} BaseWin;

/* Any changes to this structure must be reflected in axis.c
   in functions axis_create() and (probably) axis_delete()
*/
typedef struct rtg_axis {
  struct rtg_axis *next;
  BaseWin *window; /* is this necessary? */
  unsigned short min_coord;
  unsigned short max_coord;
  unsigned short n_coords;
  /* scaling functions */
  struct {
	double offset;
	double factor;
	short int shift;
  } scale;
  unsigned char auto_scale_required:1;
  unsigned char rescale_required:1;
  unsigned char redraw_required:1;
  unsigned char is_y_axis:1;
  unsigned char deleted:1;

  /* Following are the public options */
  RtgAxisOpts opt;
} RtgAxis;

/* Any changes to this structure must be reflected in graph.c in functions
   graph_create() and (probably) graph_delete()
*/
typedef struct rtg_grph {
  struct rtg_grph *next;
  BaseWin *window;
  RtgAxis *X_Axis;
  RtgAxis *Y_Axis;
  chanpos *position;
  chanpos *lookahead;
  unsigned short looked_ahead:1;

  /* Public Options */
  unsigned short line_thickness;
  unsigned short line_color;
  unsigned short line_style;
  unsigned short symbol;
} RtgGraph;

typedef struct {
  double val;
  double clip;
  unsigned short coord;
  unsigned char flag;
} clip_coord;

typedef struct {
  clip_coord X;
  clip_coord Y;
} clip_pair;

/* The "node" structure is valid if word != 0, otherwise the
   "leaf" structure is valid.
*/
typedef struct RtgCTNode {
  dastring word;
  union {
	struct {
	  struct RtgCTNode *child;
	  struct RtgCTNode *sibling;
	} node;
	struct {
	  chandef *channel;
	} leaf;
  } u;
} RtgChanNode;

/* rtg.c */
void main(void);

/* winmgr.c */
#ifdef _QEVENT_H_
  typedef int event_handler(QW_EVENT_MSG *, char *);

  void set_win_handler(int window_id, event_handler *func);
  void set_key_handler(int keyltr, event_handler *func);
#endif
void del_win_handler(int window_id);
void del_key_handler(int keyltr);
void Receive_Loop(void);

/* basewin.c */
void New_Base_Window(void);
BaseWin *BaseWin_find(char bw_ltr);
int plotting(void);
extern BaseWin *BaseWins;

/* channels.c */
void channel_opts( int key, char bw_ltr );
void channel_menu(char *title, void (* callback)(const char *, char), char bw_ltr);

/* chan_int.c */
int channels_defined(void);
chandef *channel_create(const char *name, chantype *type, int channel_id);
int channel_delete(const char *name);
chandef *channel_props(const char *name);
void Draw_channel_menu( const char *label, const char *title );
chanpos *position_create(chandef *);
chanpos *position_duplicate(chanpos *oldpos);
void position_delete(chanpos *);

/* graph.c */
void graph_create(const char *channel, char bw_ltr);
void graph_delete(RtgGraph *graph);
void lookahead(RtgGraph *graph);
void plot_graph(RtgGraph *graph);

/* axis.c */
RtgAxis *axis_create(BaseWin *bw, chandef *channel, int is_y_axis);
void axis_delete(RtgAxis *ax);
void axis_auto_range(RtgAxis *ax);
void axis_scale(RtgAxis *ax);
void axis_draw(RtgAxis *ax);
extern RtgAxisOpts *X_Axis_Opts, *Y_Axis_Opts;
dastring dastring_init(const char *new);
void dastring_update(dastring *das, const char *new);
void axopts_init(RtgAxisOpts *to, RtgAxisOpts *from);
void axopts_update(RtgAxisOpts *to, RtgAxisOpts *from);

/* clip.c */
int clip_line(RtgGraph *graph, clip_pair *p1, clip_pair *p2);

/* chan_tree.c */
RtgChanNode *ChanTree_find(const char *name);
RtgChanNode *ChanTree_insert(const char *name);
void ChanTree_delete(const char *name);
extern RtgChanNode *CT_Root;

/* dummy.c */
void dummy_channel_create(const char *name);

/* chanprop.c */
void chanprop_dialog(const char *chname);
void chanprop_delete(chandef *chan);

/* axisprop.c */
enum axprop_type { AP_CHANNEL_X, AP_CHANNEL_Y, AP_GRAPH_X, AP_GRAPH_Y, 
					AP_NTYPES };
void axisprop_dialog(enum axprop_type type, const char *name);
void axisprop_delete(enum axprop_type type);
void axisprop_update(enum axprop_type type, const char *name);
