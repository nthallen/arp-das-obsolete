#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef __QNX__
#include <sys/name.h>
#endif
#include "qnx_ipc.h"
#include "port_types.h"

int name_locate(int nid, char *name, unsigned size, unsigned *copies) {
  int i;
  BYTE4 j;
  switch (ipc) {
#ifdef __QNX__
  case MSG_QNX_IPC: return(qnx_name_locate(nid,name,size,copies)); break;
#endif
  default:
  case FIFO_QNX_IPC:
    j=strlen(ipc_dir);
    i=open(strcat(ipc_dir,*name == '/' ? name+1 : name),O_RDONLY );
    ipc_dir[j]='\0';
    if (i!=-1) {
      if (read(i,&j,sizeof(BYTE4))!=sizeof(BYTE4))
	return(-1);
    }
    close(i);
    return(j);
  }
}

