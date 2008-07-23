/* pick_file.c
 * $Log$
 * Revision 1.4  1996/03/21  14:25:52  nort
 * *** empty log message ***
 *
 * Revision 1.3  1995/06/20  15:51:49  nort
 * parent support, etc.
 *
 * Revision 1.2  1995/05/26  18:46:19  nort
 * Many mods prior to 5/26/95
 *
 * Revision 1.1  1993/09/16  19:07:19  nort
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/name.h>
#include <sys/kernel.h>
#include <sys/psinfo.h>
#include <process.h>
#include "nortlib.h"
#include "subbus.h"
#include "dbr.h"
#include "oui.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

#define F_SERVER 1
#define F_COPY 2
#define F_NOFULL 4
#define F_QUIT 8
#define F_NODEOUT 16
static int flags = F_SERVER | F_NOFULL;

#define PICKNAME "pick_file"
#define MY_LINE_MAX FILENAME_MAX
#define PF_QUERY 217
#define PF_COPY 218
#define PF_FILE 219
#define PF_ERROR 220
#define PF_OK 221
#define PF_QUIT 222
#define PF_PIGGYBACK 223
#define TMPACTION "/tmp/runfile"
#define DFLT_FILE "runfile.dflt"
#define ERRACTION "/dev/null"
/* Message formats:
 PF_QUERY
   Reply is PF_COPY or PF_FILE followed by rule
 PF_COPY reply type
 PF_FILE reply type
 PF_ERROR
   rule is the text of the error message
 PF_OK
 PF_QUIT
   node_id is defined.
 PF_PIGGYBACK
   reply is a PF_QUIT message
*/

nid_t exp_node = 0;
int serving = 1;
int name_id, uniq_name_id = -1;
nid_t FlightNode = 0;
struct pf_msg {
  unsigned char type;
  union {
	char rule[MY_LINE_MAX+1];
	nid_t node_id;
  } u;
};
#define MAX_PIGGYBACKS 10
int n_piggybacks = 0;
pid_t piggybacks[MAX_PIGGYBACKS];

void server_message(pid_t who, struct pf_msg *buf) {
  switch (buf->type) {
	case PF_QUERY:
	  buf->type = (flags & F_COPY) ? PF_COPY : PF_FILE;
	  if (Reply(who, buf, strlen(buf->u.rule)+2) == -1)
		nl_error(3, "Error replying: %s", strerror(errno));
	  who = Receive(who, buf, sizeof(struct pf_msg));
	  if (who == -1)
		nl_error(3, "Error Receiving Confirmation: %s", strerror(errno));
	  reply_byte(who, PF_OK);
	  if (buf->type == PF_ERROR)
		nl_error(3, "Error from client: %s", buf->u.rule);
	  break;
	case PF_QUIT:
	  qnx_name_detach(0, name_id);
	  if ( uniq_name_id != -1 )
		qnx_name_detach( FlightNode, uniq_name_id );
	  serving = 0;
	  if (exp_node != 0 && exp_node != buf->u.node_id)
		nl_error(3, "Inconsistent node information: %ld and %ld",
			exp_node, buf->u.node_id);
	  exp_node = buf->u.node_id;
	  if (exp_node == 0)
		nl_error(4, "Received zero node in QUIT message");

	  /* Then look for any renegade pick_files */
	  for (;;) {
		pid_t latent;
		struct pf_msg query;

		/* look for another pickfile */
		latent = qnx_name_locate( 0, nl_make_name(PICKNAME, 1), 0, NULL);
		if (latent == -1) break;
		if ( Send( latent, &buf, &query, sizeof(buf), sizeof(query) ) == 0) {
		  if (query.type != PF_OK)
			nl_error( 1, "Invalid response from latent pick_file" );
		} else nl_error( 1, "Error sending to latent pick_file" );
	  }

	  reply_byte(who, PF_OK);
	  break;
	case PF_PIGGYBACK:
	  /* save request */
	  if (n_piggybacks >= MAX_PIGGYBACKS) {
		buf->type = PF_ERROR;
		strcpy(buf->u.rule, "Too many piggyback requests");
		Reply(who, buf, strlen(buf->u.rule)+2);
	  } else piggybacks[n_piggybacks++] = who;
	  break;
	default:
	  nl_error(3, "Unknown command type %d received", buf->type);
  }
}

nid_t get_pids_nid(pid_t pid) {
  struct _psinfo psdata;
  
  if (qnx_psinfo(0, pid, &psdata, 0, NULL) != pid)
	nl_error(3, "Unable to get process info on pid %d", pid);
  if (psdata.flags & _PPF_VID) {
	/* This was a test to try to figure out if the two are different.
	   They sometimes are. The remote_nid is what we want/need */
	/*if (psdata.sid_nid != psdata.un.vproc.remote_nid)
		nl_error(1, "VC sid_nid is %ld, remote_nid is %ld",
			psdata.sid_nid, psdata.un.vproc.remote_nid); */
	return(psdata.un.vproc.remote_nid);
  } else return(getnid());
}

void pick_server(int argc, char **argv) {
  struct pf_msg buf, query;
  pid_t who;
  char *SFlightNode;
  int waiting_for_fn = 0;
  if (optind >= argc)
	nl_error(3, "Server mode requires a filename argument");
  if (!(flags & F_NOFULL)) {
	if (qnx_fullpath(buf.u.rule, argv[optind]) == NULL)
	  nl_error(3, "File %s not found", argv[optind]);
  } else strncpy(buf.u.rule, argv[optind], MY_LINE_MAX);
  buf.u.rule[MY_LINE_MAX] = '\0';

  SFlightNode = getenv( "FlightNode" );
  if ( SFlightNode != 0 && *SFlightNode != '\0' )
	FlightNode = atoi( SFlightNode );

  for (;;) {
	/* look for another pickfile */
	who = qnx_name_locate(0, nl_make_name(PICKNAME, 1), 0, NULL);
	if (who != -1) {
	  query.type = PF_PIGGYBACK;
	  if (Send(who, &query, &query, 1, sizeof(query)) == 0) {
		if (query.type == PF_ERROR)
		  nl_error(3, "%s", query.u.rule);
		else if (query.type != PF_QUIT || query.u.node_id == 0)
		  nl_error(4, "Ill-formed response to PIGGYBACK");
		exp_node = query.u.node_id;
		return;
	  }
	} else {
	  who = qnx_name_locate(0, nl_make_name(DG_NAME, 1), 0, NULL);
	  if (who != -1) {
		exp_node = get_pids_nid(who);
		return;
	  } else {
		/* attach the name */
		if ( FlightNode != 0 ) {
		  uniq_name_id = qnx_name_attach( FlightNode,
							nl_make_name( PICKNAME, 0 ));
		  if ( uniq_name_id == -1 ) {
			if ( errno == EBUSY ) continue;
			else if ( errno == EHOSTUNREACH ) {
			  /* emulate namewait here */
			  if ( ! waiting_for_fn ) {
				nl_error( 0, "Waiting for FlightNode %d...", FlightNode );
				waiting_for_fn = 1;
			  }
			  sleep( 1 );
			  continue;
			} else nl_error(3, "Error attaching name: %s", strerror(errno));
		  }
		} else uniq_name_id = -1;
		name_id = qnx_name_attach( 0, nl_make_name(PICKNAME, 1));
		if ( name_id == -1 ) {
		  if ( errno == EBUSY ) continue;
		  else nl_error(3, "Error attaching name: %s", strerror(errno));
		}
		break;
	  }
	}
  }

  /* Now we are a server */
  if ( waiting_for_fn ) nl_error( 0, "Continuing" );
  
  /* Shut down an old run if remnants are still around... */
  {	pid_t pid;
	unsigned char rv;
	unsigned short status, waiting_for_pfname = 0;

	pid = qnx_name_locate( 0, nl_make_name( "parent", 1 ), 0, 0 );
	if ( pid != -1 ) {
	  status = 'pf';
	  for (;;) {
		if ( Send( pid, &status, &rv, sizeof( status ), 1 ) != 0 ) {
		  if ( errno == ESRCH ) {
			nl_error( 1, "parent quit or died before I asked" );
			break;
		  }
		} else if ( rv == 0 ) {
		  nl_error( 0, "parent quit request acknowledged" );
		  break;
		} else if ( rv == 2 ) {
		  if ( ! waiting_for_pfname ) {
			nl_error( 0, "parent quit request deferred: will retry" );
			waiting_for_pfname = 1;
		  }
		  sleep(1);
		} else {
		  nl_error( 2, "Unexpected response from parent quit request: %d", rv );
		  break;
		}
	  }
	}
  }

  if ( FlightNode != 0 ) {
	int rc, waiting_for_con = 0;
	FILE *fp;
	char con_name[40];
	
	sprintf( con_name, "//%d/dev/con", FlightNode );
	for (;;) {
	  fp = fopen( con_name, "rw" );
	  if ( fp == 0 ) {
		if ( ! waiting_for_con ) {
		  nl_error( 0, "Waiting for free console %s", con_name );
		  waiting_for_con = 1;
		}
		sleep( 1 );
	  } else break;
	}
	fclose( fp );
	if ( waiting_for_con ) nl_error( 0, "Continuing" );
	
	rc = spawnl( P_WAIT, "/usr/bin/on", "/usr/bin/on", "-f", SFlightNode,
	  "-t", "/dev/con",	"/usr/local/bin/flight.sh", NULL );
	if ( rc == -1 )
	  nl_error( 3, "Error spawning flight.sh: %s", strerror( errno ) );
  }
  for ( ; ; ) {
	if (serving) {
	  who = Receive(0, &buf, sizeof(buf));
	  if (who == -1)
		nl_error(3, "Error Receiving: %s", strerror(errno));
	} else {
	  who = Creceive(0, &buf, sizeof(buf));
	  if (who == -1) break;
	}
	server_message(who, &buf);
  }

  /* Now service any PIGGYBACK requests */
  buf.type = PF_QUIT;
  buf.u.node_id = exp_node;
  while (n_piggybacks > 0)
	Reply(piggybacks[--n_piggybacks], &buf, sizeof(buf));
}

void check_line(struct pf_msg *buf, pid_t server) {
  char *p, was;
  int bsize, c, tocopy;
  FILE *fp, *ofp;

  tocopy = (buf->type == PF_COPY);  
  buf->type = PF_ERROR;
  for (p = buf->u.rule; *p != ' ' && *p != '\0'; p++);
  if (p == buf->u.rule) strcpy(buf->u.rule, "No Filename Found");
  else {
    was = *p;
	*p = '\0';
	fp = fopen(buf->u.rule, "r");
	if (fp == NULL) strcpy(buf->u.rule, "File not found");
	else {
	  if (tocopy) {
		ofp = fopen(TMPACTION, "w");
		if (ofp == NULL) strcpy(buf->u.rule, "Unable to open temp file");
		else {
		  for (;;) { c = getc(fp); if (c == EOF) break; putc(c, fp); }
		  fclose(ofp);
		  buf->type = PF_OK;
		  printf("%s\n", TMPACTION);
		}
	  } else {
		buf->type = PF_OK;
		printf("%s\n", buf->u.rule);
	  }
	  fclose(fp);
	}
  }
  if (buf->type == PF_OK) bsize = 1;
  else {
	bsize = strlen(buf->u.rule) + 2;
	printf("%s\n", ERRACTION);
  }
  Send(server, buf, buf, bsize, 1);
}

void pick_client(void) {
  pid_t server;
  struct pf_msg buf;
  char *srvr_name;
  int sw;
  FILE *fp;
  
  srvr_name = nl_make_name( PICKNAME, 1 );
  server = qnx_name_locate(0, srvr_name, 0, NULL);
  if (flags & F_QUIT) {
	if (server != -1) {
	  buf.type = PF_QUIT;
	  buf.u.node_id = getnid();
	  if (Send(server, &buf, &buf, sizeof(buf), sizeof(buf)) == 0) {
		if (buf.type != PF_OK)
		  nl_error(1, "Invalid response from server");
	  } else nl_error(1, "Error sending to server");
	}
	return;
  }
  if (server != -1) {
	buf.type = PF_QUERY;
	if (Send(server, &buf, &buf, 1, sizeof(buf)) == -1)
	  printf("%s\n", ERRACTION);
	else check_line(&buf, server);
  } else {
	if (load_subbus() != 0) {
	  sw = read_switches();
	  sprintf(buf.u.rule, "runfile.%c%c%c%c", (sw&8)?'1':'0', (sw&4)?'1':'0',
			(sw&2)?'1':'0', (sw&1)?'1':'0');
	  fp = fopen(buf.u.rule, "r");
	  if (fp != NULL) {
		fclose(fp);
		printf("%s\n", buf.u.rule);
		return;
	  }
	}
	fp = fopen(DFLT_FILE, "r");
	if (fp != NULL) {
	  fclose(fp);
	  printf("%s", DFLT_FILE);
	  return;
	}
	printf("%s", ERRACTION);
  }
}

void pf_init_options( int argc, char **argv ) {
  int c;
  
  opterr = 0;
  optind = 0;
  while (( c = getopt( argc, argv, opt_string )) != -1) {
	switch (c) {
	  case 'C': flags &= ~F_SERVER; break;
	  case 'c': flags |= F_COPY; break;
	  case 'p': flags &= ~F_NOFULL; break;
	  case 'q': flags |= F_QUIT; flags &= ~(F_SERVER|F_NOFULL); break;
	  case 'n': flags |= F_NODEOUT; break;
	  case 'l': nl_debug_level--; break;
	  case '?': nl_error(3, "Undefined option -%c", optopt);
	  default: nl_error(4, "Unsupported option -%c", c);
	}
  }
}

int main( int argc, char **argv ) {
  oui_init_options( argc, argv );
  if (flags & F_SERVER) pick_server(argc, argv);
  else pick_client();
  if (flags & F_NODEOUT) printf("%ld\n", exp_node);
  return(0);
}
