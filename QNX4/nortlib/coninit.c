/* coninit.c contains CON_init_options() */
#include <fcntl.h>
#include <unistd.h>
#include <sys/dev.h>
#include <sys/proxy.h>
#include "nl_cons.h"
#include "nortlib.h"
char rcsid_coninit_c[] =
	"$Header$";

static int n_cons = 1;
static int release_cons = 1;
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
	fd = open(name, O_RDWR);
	if (fd < 0)
	  nl_error(3, "Error opening console file %s", name);
	else nl_con_fd(index, fd);
	if ( release_cons )
	  getcon_release( name );
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
	  case 'g':
		release_cons = 0;
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
/*
=Name Con_init_options(): Initialization for console functions
=Subject Nortlib Console Functions
=Subject Startup
=Synopsis

#include "nl_cons.h"
void Con_init_options(int argcc, char **argvv);

=Description

  Con_init_options() processes the command line arguments for the
  Nortlib Console Functions. The two arguments supported are 'A'
  and 'a'. Both take an argument which is the name of the console
  device to open. By default, the first console is that
  associated with the standard output, but it can be redefined by
  use of the 'A' option. The 'a' option defines the second and
  all subsequent consoles. If any device argument is '-', that
  console is not opened and subsequent output to that index will
  be supressed.<P>
  
  After each console is opened, an attempt is made
  to contact getcon to release the console. The 'g' option
  supresses this release request for consoles appearing later in
  the command line. This might be appropriate for short-lived
  programs such as scrpaint (if it used nl_cons) where it is
  desirable for getcon to keep the console open.<P>
  
  At present, these functions support only QNX consoles. At some
  point in the future, they may be switched over to use terminfo
  and hence support arbitrary terminal types.

=Returns

  Nothing.

=SeeAlso

  =Nortlib Console Functions=.

=End

=Name nlcon_ctrl(): Get control structure for console
=Subject Nortlib Console Functions
=Synopsis

#include "nl_cons.h"
int nlcon_ctrl(unsigned int index, struct _console_ctrl **con_ctrl) {

=Description

  nlcon_ctrl() can be used to determine whether a specifed
  console index has been define and if so obtain access to its
  control structure.

=Returns

  Non-zero if the console index has been defined. If so, the
  pointer pointed to by the con_ctrl argument is replaced with a
  pointer to the console's control structure.

=SeeAlso

=End

=Name nlcon_close(): Close a console
=Subject Nortlib Console Functions
=Synopsis

#include "nl_cons.h"
void nlcon_close(void) {

=Description

  nlcon_close() is the polite way to close a console.

=Returns

  Nothing.

=SeeAlso

  =Nortlib Console Functions=.

=End
*/
