/* nl_cons.h defines functions required for general use of
 * QNX consoles.
 * $Log$
 * Revision 1.4  1995/10/06  16:22:31  nort
 * Modified nlcon_display() to support row, col on variable size consoles.
 *
 * Revision 1.3  1995/09/07  20:10:47  nort
 * Added rows, columns to nl_con_def struct
 *
 * Revision 1.2  1994/11/22  14:55:07  nort
 * Minor argument change.
 *
 * Revision 1.1  1993/01/09  15:51:18  nort
 * Initial revision
 *
 * $Id$
 */
#ifndef NL_CONS_INCLUDED
#define NL_CONS_INCLUDED
#include <sys/console.h>
#include "nortlib.h"

#define IBUFSZ 40
#define MAXCONS 10

typedef struct {
  int fd;
  unsigned old_mode;
  struct _console_ctrl *con_ctrl;
  nid_t nid;
  pid_t proxy;
  pid_t rproxy;
  int rows, columns;
} nl_con_def;

#define OPT_CON_INIT "a:A:"

extern nl_con_def nl_cons[MAXCONS];
extern int nlcons_defined;
extern char * opt_string;
void Con_init_options(int argcc, char **argvv); /* coninit.c */
void nlcon_display(unsigned int index, int row, int col,
				   const char *s, char attr); /* coninit.c */
int nlcon_ctrl(unsigned int index, struct _console_ctrl **con_ctrl); /* coninit.c */
int nlcon_getch(void); /* congetch.c */
void nlcg_receive(pid_t who); /* cgrecv.c */
void nlcon_close(void); /* coninit.c */

#endif
