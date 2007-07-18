#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "qnx_ipc.h"

int qnx_ipc_init(void) {
  char *e;
  static char one;
  if (one++) return(ipc);
  /* look for environment variable FIFO_QNX_IPC */
  e=getenv("FIFO_QNX_IPC");
  if (e) {
    ipc = FIFO_QNX_IPC;
    if (e[0]=='.' || e[0]=='/') {
      if (access(e,W_OK)) return(-1);
      else strncpy(ipc_dir,e,PATH_MAX);
      if (ipc_dir[strlen(ipc_dir)-1]!='/')
	strcat(ipc_dir,"/");
    }
  }
  if (ipc==FIFO_QNX_IPC) {
    if (mkfifo(qnx_ipc_tmp(getpid(),'\0',0),\
	       S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)==-1)
	return(-1);
    if (mkfifo(qnx_ipc_tmp(getpid(),'R',0),\
	       S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH)==-1)
	return(-1);
  }
  return(ipc);
}
