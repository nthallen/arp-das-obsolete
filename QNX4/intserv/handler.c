#include "intserv.h"
#include "internal.h"

pid_t far expint_handler( void ) {
  return expint_proxy;
}
