#include "intserv.h"
#include "internal.h"

pid_t far expint_handler( void ) {
  return expint_proxy;
}

pid_t far spare_handler( void ) {
  return spare_proxy;
}

pid_t far pfail_handler( void ) {
  return pfail_proxy;
}
