/* cmdalgo.h defines entry points into the cmdgen-based algorithms
 * $Log$
 */
command(void);
int command_exec(char *cmdtxt);
void command_init(void);
void command_record(int testcmds);
int command_check(void);
unsigned short command_getch(void);
void straight_line(int argc, char **argv);
void command_algo(int argc, char **argv); /* cmdalgo.c */
void timeline_text(char *s, int c); /* cmdalgo.c */
void timeline_init(void); /* cmdalgo.c */
void timeline_time(long int t, int c); /* cmdalgo.c */
void read_mode(void);
void select_mode(char *s);

extern signed long time_now; /* current time */
extern signed long time_prev; /* last time we looked */
extern signed long time_mode; /* elapsed time in present mode */
extern signed long time_first; /* starting time of all algorithms */
extern signed long time_next; /* seconds until next command */
extern signed long time_run; /* seconds into the total run */
extern int holding; /* true if not proceeding */
extern int cur_mode;

#define OPT_CMDALGO "t"

#define MAX_MODES 20
