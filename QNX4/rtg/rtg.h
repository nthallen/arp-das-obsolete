/* rtg.h definitions for rtg
 * $Log$
 */
 
#define SRCDIR "/usr/local/src/das/rtg/"

typedef int event_handler(QW_EVENT_MSG *, char *);

typedef struct chan_def {
  struct chan_def *next;
  const char *name;
  const char *units;
  struct cdb_str *cdb;
} chandef;

typedef struct bwstr {
  struct bwstr *next;
  int wind_id;
  int pict_id;
  int bw_id; /* unique ID number */
  char bw_label[3]; /* 'A' + bw_id */
  struct rtg_grph *graphs;
  struct rtg_axis *axes;
  struct rtg_trig *triggers;
  unsigned char resize_required:1;
  unsigned char redraw_required:1;

  /* Following are the public options */
  const char *title;
  int bkgd_color;
} BaseWin;

typedef struct rtg_axis {
  struct rtg_axis *next;
  BaseWin *window;
  unsigned short min_coord;
  unsigned short max_coord;
  /* scaling functions */
  unsigned char auto_scale_required:1;
  unsigned char rescale_required:1;

  /* Following are the public options */
  const char *units;
  double min_limit;
  double max_limit;
  double min_obsrvd;
  double max_obsrvd;
  unsigned char min_auto:1;
  unsigned char max_auto:1;
  unsigned char scope:1;
  unsigned char scroll:1;
  unsigned char normal:1;
  unsigned char single_sweep:1;
  unsigned char clear_on_trig:1;
  /* labeling options { none for now } */
} RtgAxis;

typedef struct rtg_grph {
  RtgAxis X_Axis;
  RtgAxis Y_Axis;
  chandef *channel;
  struct cdb_pos_str *pos;
  
  /* Public Options */
  unsigned short line_thickness;
  unsigned short line_color;
  unsigned short line_style;
  unsigned short symbol;
} RtgGraph;

/* rtg.c */
void main(void);

/* winmgr.c */
void set_win_handler(int window_id, event_handler *func);
void del_win_handler(int window_id);
void set_key_handler(int keyltr, event_handler *func);
void del_key_handler(int keyltr);
void Receive_Loop(void);

/* basewin.c */
void New_Base_Window(void);
BaseWin *BaseWin_find(char bw_ltr);

/* channels.c */
void channel_opts( int key, char bw_ltr );
void channel_menu(char *title, void (* callback)(const char *, char), char bw_ltr);

/* chan_int.c */
int channels_defined(void);
int channel_create(const char *name);
int channel_delete(const char *name);
chandef *channel_props(const char *name);
void Draw_channel_menu( const char *label, const char *title );

/* graph.c */
void graph_create(const char *channel, char bw_ltr);
