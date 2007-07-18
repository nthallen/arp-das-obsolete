#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "fd.h"
#include "port_types.h"

/* BLOCKING read */
/* We're not gonna return partial transfers, and make the assumption
   that if we were able to read some data and got interupted, we won't
   block reading the rest of the data.
   Return -1 or number of bytes in msg read.
*/
int fd_rdmx(int fd, unsigned parts, struct _mxfer_entry far *msgmx) {
  int byts, i,j,k;
  char *buf,*b;
  BYTE2 mbyts;
  /* read 2 byte number of bytes */
  i=j=0;
  do {
    j=read(fd,(&mbyts)+i,2-i);
    if (j==0) return(0);
    if (j==-1) {
      if (errno != EINTR) break;
      if (i==0) break;
      errno=0;
      j=0;
    }
    i+=j;
  } while (i!=2);
  if (j!=2) return(-1);
  if (mbyts==0) return(0);

  if ((buf=malloc(mbyts)) == NULL) return(-1);

  i=j=0;
  while (i!=mbyts) {
    j=read(fd,buf+i,mbyts-i);
    if (j==0) return(-1); /* this shouldn't happen */
    if (j==-1) {
      if (errno != EINTR) break;
      errno = 0;
      j=0;
    }
    i+=j;
  }

  if (i==mbyts)
    for (i=0, k=0, b=buf;
	 i<parts && k<mbyts;
	 k+=msgmx[i].mxfer_len, b+=msgmx[i].mxfer_len,i++)
      memcpy(msgmx[i].mxfer_off,b,
	     (mbyts-k)<msgmx[i].mxfer_len ? mbyts-k : msgmx[i].mxfer_len);

  free(buf);
  return(j==0 ? 0 : j != -1 ? mbyts : -1);
}
