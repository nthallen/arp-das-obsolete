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
  int i,j;
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
    j=strlen(ipc_dir);
    sprintf(ipc_dir,"%s/%d",ipc_dir,getpid());
    fifo_in=open(ipc_dir,O_RDONLY|O_CREAT|O_EXCL,S_IRUSR|S_IRGRP|S_IROTH);
    sprintf(ipc_dir,"%sR",ipc_dir);
    fifo_r=open(ipc_dir,O_RDONLY|O_CREAT|O_EXCL,S_IRUSR|S_IRGRP|S_IROTH);
    ipc_dir[j]='\0';
    if (fifo_in==-1 || fifo_r==-1) return(-1);
  }
  return(ipc);
}
