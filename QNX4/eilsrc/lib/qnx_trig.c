#include <sys/types.h>
#ifdef __QNX__
#include <sys/kernel.h>
#include <sys/proxy.h>
#endif
#include "qnx_ipc.h"
#include "fd.h"

int qnx_trig(pid_t proxy, char *data, int nbytes) {
  int b;
  switch (ipc) {
#ifdef __QNX__
  case MSG_QNX_IPC: return(Trigger(proxy)); break;
#endif
  case FIFO_QNX_IPC:
    /* non blocking write to proxy */
    b = fd_is_block(STDOUT_FILENO);
    if (b) fd_unblock(STDOUT_FILENO);
    snd(proxy, data, 0, nbytes, 0);
    if (b) fd_block(STDOUT_FILENO);
    break;
    default : break;
  }
  return 0;
}

int proxy_attach(int pid, char *data, int nbytes, int priority, int flag) {
	switch (flag) {
#ifdef __QNX__
		case IPC_QNX: return(qnx_proxy_attach(pid, data, nbytes, priority));
#endif
		case IPC_FIFO: return(pid);
		default: return 0; 
	}
}

int proxy_detach(int pid, int flag) {
	switch (flag) {
#ifdef __QNX__
		case IPC_QNX: return(qnx_proxy_detach(pid));
#endif
		case IPC_FIFO:
		default: return 0; 
	}	
}
