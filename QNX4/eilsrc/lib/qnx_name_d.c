#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef __QNX__
#include <sys/name.h>
#endif
#include "qnx_ipc.h"

int qnx_name_detach(int nid, int name_id, char *name) {
  int i,j;
  switch (ipc) {
#ifdef __QNX__
  case MSG_QNX_IPC: return(qnx_name_detach(nid,name_id));
#endif
  default:
  case FIFO_QNX_IPC:
    if (name==NULL || *name=='\0') return(-1);
    j=strlen(ipc_dir);
    i=remove(strcat(ipc_dir,*name == '/' ? name+1 : name));
    ipc_dir[j]='\0';
    return(i!=0 ? -1 : 0);
  }
}
