#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <string.h>
#include <unistd.h>
#include "nortlib.h"
#include "company.h"

char *getcon_server_name( pid_t ppid ) {
  static char name[80];
  sprintf( name, "%s.%d", COMPANY "/getcon", ppid );
  return name;
}

static pid_t getcon_pid;
/* getcon_release() requests that the resident
   getcon release the specified console, basically
   taking over responsibility for
   keeping that console open. Returns non-zero on
   success. Failure is considered non-fatal and
   will not generate error messages. If conname is
   "Q", resident getcon will quit.
*/
int getcon_release( char *conname ) {
  if ( getcon_pid <= 0 ) {
	getcon_pid = qnx_name_locate( getnid(),
	  getcon_server_name(getppid()), 0, NULL );
  }
  if ( getcon_pid > 0 ) {
	if ( ! Send( getcon_pid, conname, 0, strlen(conname)+1, 0 ) )
	  return 1;
  }
  return 0;
}
