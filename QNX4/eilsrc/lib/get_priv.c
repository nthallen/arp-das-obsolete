#ifdef __QNX__
#include <i86.h>
int get_priv() {
struct SREGS regs;
segread(&regs);
return(regs.cs & 0x03);
}
#endif
