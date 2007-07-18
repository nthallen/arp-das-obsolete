#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <stdlib.h>
#include "mx.h"
#include "port_types.h"

/* BLOCKING write */
/* We're not gonna return partial transfers, and make the assumption
   that if we were able to write some data and got interupted, we won't
   block writing the rest of the data.
   Return -1 or number of bytes in msg.
*/
int fd_wrmx(int fd, unsigned sparts, struct _mxfer_entry far *smsg) {
  char *sbuf;
  int i,j;
  BYTE2 sbytes;
  for (i=0,sbytes=0; i<sparts; i++) sbytes+=smsg[i].mxfer_len;
  if ((sbuf=malloc(sbytes+2)) == NULL) 
    return(-1);
  memcpy(sbuf,&sbytes,2);
  for (i=0,sbytes=0; i<sparts; sbytes+=smsg[i].mxfer_len,i++)
    memcpy(sbuf+sbytes+2,smsg[i].mxfer_off,smsg[i].mxfer_len);
  i=j=0;
  do {
    j=write(fd,sbuf+i,sbytes+2-i);
    if (j==-1) {
      if (errno!=EINTR) break;
      if (i==0) break;
      errno=0;
      j=0;
    }
    i+=j;
  } while (i!=sbytes+2);
  free(sbuf);
  return(j != -1 ? i-2 : -1);
}
