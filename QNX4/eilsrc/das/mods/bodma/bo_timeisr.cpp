#include <sys/types.h>

volatile long tcks;
long tick_w;
pid_t far my_ticks ()
{
	tcks = (tcks + 1) % tick_w;
	return 0;
}
