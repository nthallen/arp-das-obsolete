#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include "mx.h"

int fd_rdmx(int fd, unsigned parts, struct _mxfer_entry __far *msgmx) {
  int byts, i,j;
  char *buf;
  for (i=0,byts=0;i<parts;i++) byts+=msgmx[i]->mxfer_len;
  if ((buf=malloc(byts))==-1) return(-1);
  if ( (j=read(fd,buf,byts))==0)
    for (i=0; i<parts;buf+=msgmx[i]->mxfer_len,i++)
      memcpy(msgmx[i]->mxfer_off,buf,msgmx[i]->mxfer_len);
  free(buf);
  return(j);
}  
}
