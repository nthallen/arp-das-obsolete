#include "nortlib.h"
#include "company.h"

char *getcon_server_name( pid_t ppid ) {
  static char name[80];
  sprintf( name, "%s.%d", COMPANY "/getcon", ppid );
  return name;
}
