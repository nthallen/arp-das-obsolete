/* coninit.c contains CON_init_options()
 * $Log$
 * Revision 1.2  1993/07/01  15:35:04  nort
 * Eliminated "unreferenced" via Watcom pragma
 *
 * Revision 1.1  1993/02/18  02:29:36  nort
 * Initial revision
 *
 */
#include <fcntl.h>
#include <unistd.h>
#include <sys/dev.h>
#include <sys/proxy.h>
#include "nl_cons.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

#define CDWIDTH 160

static int n_cons = 1;
nl_con_def nl_cons[MAXCONS];
int nlcons_defined = 0;

static void nl_con_fd(int index, int fd) {
  nl_con_def *nlcd;
  
  nlcd = &nl_cons[index];
  nlcd->fd = fd;
  if ((nlcd->con_ctrl = console_open(fd, O_RDWR)) == NULL)
	nl_error(3, "Error opening console index %d", index);
  else {
	console_size( nlcd->con_ctrl, 0, 0, 0, &nlcd->rows, &nlcd->columns );
  }
  nlcons_defined++;
}

static void nl_con_file(int index, char *name) {
  int fd;

  if (*name == '-') nl_cons[index].fd = -1;
  else {
	fd = open(optarg, O_RDWR);
	if (fd < 0)
	  nl_error(3, "Error opening console file %s", optarg);
	else nl_con_fd(index, fd);
  }
}

void Con_init_options(int argcc, char **argvv) {
  int c;

  optind = 0;
  opterr = 0;
  if (argcc > 0) do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
	  case 'A':
		nl_con_file(0, optarg);
		break;
	  case 'a':
		if (n_cons == MAXCONS)
		  nl_error(4, "Too many consoles specified on command line");
		else nl_con_file(n_cons++, optarg);
		break;
	  default:
		break;
	}
  } while (c!=-1);
  opterr = 1;

  if (nl_cons[0].fd != -1 && nl_cons[0].con_ctrl == NULL)
	nl_con_fd(0, STDOUT_FILENO);
}

/* returns TRUE if console is defined */
int nlcon_ctrl(unsigned int index, struct _console_ctrl **con_ctrl) {
  if (index < n_cons)
	return((*con_ctrl = nl_cons[index].con_ctrl) != NULL);
  return(0);
}

/* displays without moving cursor. Displays a max of CDWIDTH/2 chars. */
void nlcon_display(unsigned int index, int offset,
					const char *s, char attr) {
  struct _console_ctrl *con_ctrl;
  
  if (nlcon_ctrl(index, &con_ctrl)) {
	char buf[CDWIDTH];
	int i;

	for (i = 0; *s != '\0' && i < CDWIDTH; s++) {
	  buf[i++] = *s;
	  buf[i++] = attr;
	}
	console_write(con_ctrl, 0, offset, buf, i, NULL, NULL, NULL);
  }
}

void nlcon_close(void) {
  nl_con_def *con;
  
  while (n_cons > 0) {
    con = &nl_cons[--n_cons];
	if (con->con_ctrl != NULL) {
	  if (con->rproxy != 0) {
		qnx_proxy_rem_detach(con->nid, con->rproxy);
		qnx_proxy_detach(con->proxy);
		dev_mode(con->fd, con->old_mode, _DEV_MODES);
	  }
	  close(con->fd);
	  con->con_ctrl = NULL;
	}
  }
}
