/* nl_cons.h defines functions required for general use of
 * QNX consoles.
 * $Log$
 * $Id$
 */
#ifndef NL_CONS_INCLUDED
#define NL_CONS_INCLUDED
#include <sys/console.h>
#include "nortlib.h"

#define IBUFSZ 40
#define MAXCONS 4

typedef struct {
  int fd;
  unsigned old_mode;
  struct _console_ctrl *con_ctrl;
  nid_t nid;
  pid_t proxy;
  pid_t rproxy;
} nl_con_def;

#define OPT_CON_INIT "a:A:"

extern nl_con_def nl_cons[MAXCONS];
extern int nlcons_defined;
extern char * opt_string;
void Con_init_options(int argcc, char **argvv); /* coninit.c */
void nlcon_display(unsigned int index, int offset, char *s, char attr); /* coninit.c */
int nlcon_ctrl(unsigned int index, struct _console_ctrl **con_ctrl); /* coninit.c */
int nlcon_getch(void); /* congetch.c */
void nlcg_receive(pid_t who); /* cgrecv.c */
void nlcon_close(void); /* coninit.c */

#endif
