/* rtg.h definitions for rtg
 * $Log$
 * Revision 1.9  1995/01/11  20:36:07  nort
 * During move from props.c to proper.c
 *
 * Revision 1.7  1994/12/20  20:54:32  nort
 * *** empty log message ***
 *
 * Revision 1.6  1994/12/19  16:40:27  nort
 * *** empty log message ***
 *
 * Revision 1.5  1994/12/13  16:10:01  nort
 * Realtime!
 *
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
/* #define SRCDIR "/usr/lib/windows/apps/rtg/" */

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
  unsigned char overlay;
  unsigned char min_auto;
  unsigned char max_auto;
  unsigned char scope;
  unsigned char scroll;
  unsigned char normal;
  unsigned char single_sweep;
  unsigned char clear_on_trig;
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
  struct {
	dastring X;
	dastring Y;
  } units;
} chandef;

typedef struct rtg_chantype {
  void (* channel_delete)(chandef *);
  int (* position_create)(chandef *);
  int (* position_duplicate)(chanpos *);
  void (* position_delete)(chanpos *);
  int (* position_rewind)(chanpos *);
  int (* position_data)(chanpos *, double *X, double *Y);
  int (* position_move)(chanpos *, long int index);
} chantype;

/* Any changes to this structure must be reflected in the create
   and/or delete routines in basewin.c
*/
typedef struct bwstr {
  struct bwstr *next;
  int wind_id;
  int pict_id;
  int bw_id; /* unique ID number */
  char bw_name[10]; /* "RTG_%d" serially to name picts and windows */
  char bw_label[3]; /* 'A' + bw_id */
  int row, col; /* used when reopening only... */
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
  unsigned char title_bar;
  unsigned char fix_front;
} BaseWin;

/* Any changes to this structure must be reflected in axis.c
   in functions axis_create() and (probably) axis_delete()
*/
typedef struct rtg_axis {
  struct rtg_axis *next;
  BaseWin *window; /* is this necessary? Yes */
  dastring ctname;
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
  dastring name;
  unsigned short line_thickness;
  unsigned short line_color;
  unsigned short line_style;
  dastring symbol;
  unsigned short symbol_color;
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
	union {
	  chandef *channel;
	  BaseWin *bw;
	  RtgGraph *graph;
	  RtgAxis *axis;
	  void *voidptr;
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
void basewin_close(BaseWin *bw);
int plotting(void);
extern BaseWin *BaseWins;

/* chan_int.c */
int channels_defined(void);
chandef *channel_create(const char *name, chantype *type, int channel_id,
				const char *xunits, const char *yunits);
int channel_delete(const char *name);
chandef *channel_props(const char *name);
chanpos *position_create(chandef *);
chanpos *position_duplicate(chanpos *oldpos);
void position_delete(chanpos *);

/* graph.c */
void graph_crt(BaseWin *bw, chandef *cc, RtgAxis *x_ax, RtgAxis *y_ax);
void graph_create(const char *channel, char bw_ltr);
void graph_delete(RtgGraph *graph);
void graph_ndelete(const char *name, char unrefd);
void graph_nprops(const char *name, char unrefd);
void lookahead(RtgGraph *graph);
void plot_graph(RtgGraph *graph);

/* axis.c */
void axis_ctname(RtgAxis *ax);
RtgAxis *axis_create(BaseWin *bw, const char *units, int is_y_axis);
void axis_delete(RtgAxis *ax);
void axis_auto_range(RtgAxis *ax);
void axis_scale(RtgAxis *ax);
void axis_draw(RtgAxis *ax);
extern RtgAxisOpts *X_Axis_Opts, *Y_Axis_Opts;
dastring dastring_init(const char *new);
void dastring_update(dastring *das, const char *new);
const char *dastring_value(dastring das);
void axopts_init(RtgAxisOpts *to, RtgAxisOpts *from);
void axopts_update(RtgAxisOpts *to, RtgAxisOpts *from);
const char *trim_spaces(const char *str);

/* clip.c */
int clip_line(RtgGraph *graph, clip_pair *p1, clip_pair *p2);

/* chan_tree.c */
typedef enum { CT_CHANNEL, CT_GRAPH, CT_WINDOW, CT_AXIS,
	CT_N_TREES } treetype;
RtgChanNode *ChanTree(int act, treetype tree, const char *name);
char *ChanTreeWild(treetype tree, const char *format);
int ChanTree_defined(treetype tree);
void Draw_ChanTree_Menu(treetype tree, const char *label, const char *title);
int ChanTree_Rename(treetype tree, const char *oldname, const char *newname);
#define CT_FIND 0
#define CT_INSERT 1
#define CT_DELETE 2

/* channels.c */
void channel_opts( int key, char bw_ltr );
void ChanTree_Menu( treetype tree, char *title,
	  void (* callback)(const char *, char), char bw_ltr);

/* dummy.c */
void dummy_channel_create(const char *name);

/* chanprop.c */
void chanprop_dialog(const char *chname);
void chanprop_delete(chandef *chan);

/* axisprop.c */
enum axprop_type { AP_GRAPH_X, AP_GRAPH_Y, AP_NTYPES };
void axisprop_dialog(enum axprop_type type, const char *name);
void axisprop_delete(enum axprop_type type);
void axisprop_update(enum axprop_type type, const char *name);

/* props.c */
enum proptypes { GRAPH_PROPS, N_PROPTYPES };
void Properties(const char *name, enum proptypes proptype);
void PropCancel(const char *name, enum proptypes proptype);
void PropUpdate(const char *name, enum proptypes proptype);

#ifdef _QEVENT_H_
  typedef struct {
	const char *pict_file; /* filename of the dialog picture */
	int pict_id;           /* The current picture id, starts at 0 */
	const char *pict_name; /* The picture name, beginning with '$' */
	const char *di_label;  /* The dialog label, beginning with 'p' */
	const char *di_title;  /* The dialog title */
	int (*prop2dial)(const char *name, enum proptypes proptype);
	  /* prop2dial copies appropriate prop information into the
		 dialog. It assumes the picture is current. The proptype
		 is included for dialogs which share this function with
		 other dialogs (axis props, for example). Other such
		 functions will not need to use that arg.
		 If name is NULL, dialog should be updated with the
		 properties from the current object.
		 Returns 0 on success, 1 on failure. May call nl_error.
	  */
	int (*dial2prop)(char *tag, enum proptypes proptype);
	  /* dial2prop updates the new properties structure based on
		 the single element tag. The tagged element is the Current
		 element, so ElementNumber(), ElementText() etc. will
		 return the appropriate value. As before, proptype
		 may be ignored.
	  */
	int (*apply)(enum proptypes proptype);
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
	int (*handler)(QW_EVENT_MSG *msg, enum proptypes proptype);
	  /* Auxilliary handler routine. QW_DISMISS and key 'Y'
		 are handled first (so not passed to handler)
		 Returns 1 if the message is handled, 0 otherwise.
		 QW_CLOSED and QW_CANCELLED are handled afterward
		 if handler doesn't. May be NULL.
	  */
	int (*cancel)(const char *name, enum proptypes proptype);
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
  } RtgPropDef;
#endif

/* proper.c */
void Properties_(const char *name, const char *plabel, int open_dialog);
void PropCancel_(const char *name, const char *plabel, const char *options);
void PropUpdate_(const char *name, const char *plabel);
void PropChange_(const char *plabel, const char *tag, const char *value);
int PropsApply_(const char *prop_label);
#ifdef _READ
  /* I'm guessing _READ will be a pretty portable way of knowing that
     stdio.h has been included, and hence that FILE is defined
  */
  void PropsOutput_(FILE *fp, const char *name, const char *plabel);
#endif

typedef union {
  dastring text;
  double real;
  long int long_int;
  unsigned short int ushort_int;
  short int short_int;
  unsigned char boolean;
} RtgPropValue;

/* RtgPropEltTypeDef defines a Property Element Type via function pointers
   The check and recover elements may be NULL, all others must be supplied.
*/
typedef struct {
  void (*assign)(RtgPropValue *to, RtgPropValue *from);
  void (*val2pict)(const char *tag, RtgPropValue *nv);
  void (*elt2val)(RtgPropValue *nv);
  int (*compare)(RtgPropValue *old, RtgPropValue *new);
  int (*check)(struct PropDefB *PDB, RtgPropValue *old, RtgPropValue *new);
  void (*recover)(struct PropDefB *PDB, RtgPropValue *old, RtgPropValue *new);
  void (*ascii2val)(RtgPropValue *nv, const char *str);
} RtgPropEltTypeDef;

typedef struct {
  const char *tag;
  RtgPropEltTypeDef *type;
  unsigned short offset;
} RtgPropEltDef;

typedef struct {
  RtgPropValue val;
  unsigned char changed;
} RtgPropValDef;

#ifdef _QEVENT_H_
  typedef struct PropDefB {
	struct PropDefA *def;
	int pict_id;           /* The current picture id, starts at 0 */
	void *prop_ptr;        /* The prop structure currently being edited */
	RtgPropValDef *newvals;
	int n_elements;
	int last_element;
  } RtgPropDefB;

  typedef struct PropDefA {
	const char *pict_file; /* filename of the dialog picture */
	const char *pict_name; /* The picture name, beginning with '$' */
	const char *di_label;  /* The dialog label, beginning with 'r' */
	const char *di_title;  /* The dialog title */
	void * (* find_prop)(const char *name, RtgPropDefB *prop_def);
	treetype tree; /* ChanTree in which to look for property structure */
	int (* dial_update)(RtgPropDefB *prop_def);
	int (*handler)(QW_EVENT_MSG *msg, RtgPropDefB *prop_def);
	int (* apply)(RtgPropDefB *prop_def);
	void (* applied)(RtgPropDefB *prop_def);
	int (* cancel)(RtgPropDefB *prop_def);
	RtgPropEltDef *elements;
  } RtgPropDefA;
#endif

/* elttype.c */
extern RtgPropEltTypeDef pet_string;
extern RtgPropEltTypeDef pet_key_string;
extern RtgPropEltTypeDef pet_boolean;
extern RtgPropEltTypeDef pet_exclusive;
extern RtgPropEltTypeDef pet_numus;
extern RtgPropEltTypeDef pet_numreal;
extern RtgPropEltTypeDef pet_nop;
