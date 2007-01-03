#include <sys/types.h>

pid_t do_proxy;
pid_t far isr ()
{
  return( do_proxy );
}
