/* bomem.h definitions for bomem rtg program
 * $Log$
 */
 
#define SRCDIR "/usr/local/src/das/rtg/"
#define BOMEM_MSG_MAX 50

typedef int event_handler(QW_EVENT_MSG *);

/* bomem.c */
void Initialize_DSP(void);
void main(int argc, const char * const *argv);
extern int log_data, collect_spec, spec_collected;
extern int windows; /* set nonzero if windows server is located */

/* winmgr.c */
void set_win_handler(int window_id, event_handler *func);
void del_win_handler(int window_id);
void set_key_handler(int keyltr, event_handler *func);
void del_key_handler(int keyltr);
void Receive_Loop(void);

/* basewin.c */
void New_Base_Window(void);
void update_sequence(void);
void update_n_scans(void);
void update_int_spec(void);
void dim_acquire(int dim);
extern int base_wind_id, base_pict_id;
extern int sequence, n_scans;
extern int plot_dir;
extern int plot_chan;
extern int plot_imag;

/* plot.c */
void get_pane_size(void);
void new_plot(float huge *buffer, long int npts);
int plotting(void);
void replot(void);
void sub_plot(short row, short width);

/* acquire.c */
void acquire_data(void);
void plot_opt(void);

/* ratio.c */
#ifdef BOMEM_USEFUL
  void ratio_regions(YDATA *spec_r, YDATA *spec_i);
#endif

/* server.c */
void server_command(pid_t from, char *cmd);
void server_loop(void);
