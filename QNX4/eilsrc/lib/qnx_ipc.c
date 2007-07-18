#include <limits.h>
#include "qnx_ipc.h"
#ifndef __QNX__
int ipc = FIFO_QNX_IPC;
#else
int ipc = MSG_QNX_IPC;
#endif
char ipc_dir[PATH_MAX]="/tmp";
int b_fifo_in=0;
int b_fifo_out=0;
int relay_pid=0;
int fifo_in=-1;


