/* cmdalgo.h defines entry points into the cmdgen-based algorithms
 * $Log$
 * Revision 1.2  1993/01/26  20:55:44  nort
 * Partial changes for new algorithms
 *
 * Revision 1.1  1993/01/09  15:51:16  nort
 * Initial revision
 *
 */
void cmd_init(void);
void cmd_interact(void);
int cmd_batch(char *cmd, int test);
typedef struct {
  unsigned short state;
  unsigned short value;
} cmd_state;
void cmd_report(cmd_state *s);
int cmd_check(cmd_state *s);
void cis_initialize(void);
void cis_terminate(void);
#define CMDREP_QUIT 1000
#define CMDREP_SYNERR 2000
#define CMDREP_EXECERR 3000
#define CMDREP_TYPE(x) ((x)/1000)

/* Message-level definition of command interpreter interface: */
#define CMDINTERP_NAME "cmdinterp"
#define CMD_INTERP_MAX 256
#define CMD_VERSION_MAX 80
#define CMD_PREFIX_MAX 9
typedef struct {
  unsigned char msg_type;
  char prefix[CMD_PREFIX_MAX];
  char command[CMD_INTERP_MAX];
} ci_msg;
typedef struct {
  unsigned char msg_type;
  char version[CMD_VERSION_MAX];
} ci_ver;
#define CMDINTERP_QUERY 255
#define CMDINTERP_SEND 254
#define CMDINTERP_TEST 253
#define CMDINTERP_QUIT 252
#define CMDINTERP_SEND_QUIET 251

void command_algo(int argc, char **argv); /* cmdalgo.c */
void timeline_text(char *s, int c); /* cmdalgo.c */
void timeline_init(void); /* cmdalgo.c */
void timeline_time(long int t, int c); /* cmdalgo.c */
void read_mode(void);
void select_mode(char *s);
int mode_number(char *name); /* cmdalgo.c */

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
