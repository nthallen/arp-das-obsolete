/* cmdprox.c contains get_server_proxy()
  pid_t get_server_proxy(const char *name, int global, const char *command);

  name is the name the  server registers with the OS (say "bomem")

  global is the flag passed to nl_make_name() when expanding the name to 
  a fully qualified name. Note that at the present time, the name search 
  will be strictly local, even if the global flag is specified. If 
  non-local registration is required, this functionality can be expanded 
  without affecting current applications. (Actually, it is up to the
  server to supply this functionality!)

  command is an ASCIIZ string which will is an appropriate registration 
  request to the specified server. For bomem, for example, commands 
  beginning with "boR" will register commands. The server will perform 
  the appropriate action, create a proxy and return it.
*/
#include <sys/types.h>
#include <sys/kernel.h>
#include <unistd.h>
#include <string.h>
#include "nortlib.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)

pid_t get_server_proxy(const char *name, int global, const char *cmd) {
  pid_t srvr_pid;
  int rv;
  struct {
	unsigned short status;
	pid_t proxy;
  } rep;

  srvr_pid = nl_find_name(getnid(), nl_make_name(name, global));
  if (srvr_pid != -1) {
	rv = Send(srvr_pid, cmd, &rep, strlen(cmd)+1, sizeof(rep));
	if (rv == -1) {
	  if (nl_response) nl_error(nl_response,
		  "Unable to connect to Server %s at pid %d", name, srvr_pid);
	} else if (rep.status != 0) {
	  if (nl_response) nl_error(nl_response,
		  "Server %s returned error %d", name, rep.status);
	} else return rep.proxy;
  }
  return -1;
}
