#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef __QNX__
#include <sys/name.h>
#endif
#include "qnx_ipc.h"
#include "port_types.h"

int qnx_name_attach(int nid, char *name) {
  int i;
  BYTE4 j;
  switch (ipc) {
#ifdef __QNX__
  case MSG_QNX_IPC: return(qnx_name_attach(nid,name)); break;
#endif
  default:
  case FIFO_QNX_IPC:
    if (name==NULL || *name=='\0') return(-1);
    j = strlen(ipc_dir);
    i=open(strcat(ipc_dir,*name == '/' ? name+1 : name), \
		O_WRONLY|O_CREAT|O_EXCL, \
                S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    ipc_dir[j]='\0';
    if (i!=-1) {
      j=getpid();
      if (write(i,&j,sizeof(BYTE4))!=sizeof(BYTE4))
	return(-1);
    }
    close(i);
    return(i);
  }
}

