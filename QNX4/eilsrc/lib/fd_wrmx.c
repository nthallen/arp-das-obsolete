#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include "mx.h"

int fd_wrmx(int fd, unsigned sparts, struct _mxfer_entry __far *smsg) {
  char *sbuf;
  innt sbytes, i,j;
  for (i=0,sbytes=0; i<sparts; i++) sbytes+=smsg[i]->mxfer_len;
  if ( (sbuf=malloc(sbytes))==0) return(-1);
  for (i=0,sbytes=0; i<sparts; sbytes+=smsg[i]->mxfer_len,i++)
    memcpy(sbuf+sbytes,smsg[i]->mxfer_off,smsg[i]->mxfer_len);
  j=write(fd,sbuf,sbytes);
  free(sbuf);
  return(j);
}
