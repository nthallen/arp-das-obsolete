#include <signal.h>
#include "subbus.h"
char rcsid_subbus_sbsnload_c[] =
  "$Header$";

void far sbsnload(void) { raise(SIG_NOSLIB); }
