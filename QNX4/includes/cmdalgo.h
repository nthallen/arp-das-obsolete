/* cmdalgo.h defines entry points into the cmdgen-based algorithms
 * $Log$
 * Revision 1.1  1993/01/09  15:51:16  nort
 * Initial revision
 *
 */
void cmd_init(void);
void cmd_interact(void);
void cmd_batch(char *cmd, int test);
void cmd_report(cmd_state *s);
int cmd_check(void);
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
