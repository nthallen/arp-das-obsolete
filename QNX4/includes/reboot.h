#include <stdlib.h>
#ifdef __QNX__
#define REBOOTSYS system("shutdown -f please")
#else
#define REBOOTSYS system("shutdown")
#endif
