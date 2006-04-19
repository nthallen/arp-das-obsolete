/* sendcmd.c
   Utility to send a command to an instrument from
   the commandline.
   
   sendcmd <Experiment> <command>
   The <command> should be quoted.
   
*/
#include <env.h>
#include <sys/types.h>
#include <sys/psinfo.h>
#include <unistd.h>
#include "nortlib.h"
#include "cmdalgo.h"

char *opt_string = "C:h:";
char ci_version[] = "";

static nid_t get_pids_nid(pid_t pid) {
  struct _psinfo psdata;
  
  if (qnx_psinfo(0, pid, &psdata, 0, NULL) != pid)
	nl_error(3, "Unable to get process info on pid %d", pid);
  if (psdata.flags & _PPF_VID)
	return(psdata.un.vproc.remote_nid);
  else return(getnid());
}

int main( int argc, char **argv ) {
  pid_t dg_pid;
  int dg_nid;
  char *cis_argv[3] = { "sendcmd", "-C", 0 };
  char dg_node_txt[6];
  
  if ( argc != 3 )
    nl_error( 3, "Usage: sendcmd <Experiment> <command>" );
  if ( setenv( "Experiment", argv[1], 1 ) )
    nl_error( 3, "Error from setenv" );
  dg_pid = nl_find_name(0, nl_make_name("dg", 1));
  dg_nid = get_pids_nid(dg_pid);
  sprintf( dg_node_txt, "%d", dg_nid );
  cis_argv[2] = dg_node_txt;
  cic_options( 3, cis_argv, "sendcmd:" );
  ci_sendfcmd( 0, "%s\n", argv[2] );
  return 0;
}
