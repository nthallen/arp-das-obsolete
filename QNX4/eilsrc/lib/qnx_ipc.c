#include <limits.h>
#ifndef __QNX__
unsigned char ipc = FIFO_QNX_IPC;
#else
unsigned char = MSG_QNX_IPC;
#endif
char ipc_dir[PATH_MAX]="/tmp";
int fifo_in=-1;
int fifo_out=-1;
int fifo_r=-1;
