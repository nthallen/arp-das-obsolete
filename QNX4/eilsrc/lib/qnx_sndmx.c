#ifdef __QNX__
#include <sys/kernel.h>
#else
#include <malloc.h>
#include <string.h>
#endif
#include "mx.h"
#include "qnx_ipc.h"

int sndmx(int pid, unsigned sparts, unsigned rparts,
          struct _mxfer_entry __far *smsg,
          struct _mxfer_entry __far *rmsg) {
  char *sbuf, *rbuf;
  int sbytes, rbytes, i,j;

  switch (ipc) {
#ifdef __QNX__
  case MSG_QNX_IPC: return(Sendmx(pid, sparts, rparts, smsg, rmsg)); break;
#endif
  default:
  case FIFO_QNX_IPC:
    for (i=0,sbytes=0; i<sparts; i++) sbytes+=smsg[i]->mxfer_len;
    for (i=0,rbytes=0; i<rparts; i++) rbytes+=rmsg[i]->mxfer_len;
    if ( (sbuf=malloc(sbytes))==0) return(-1);
    if ( (rbuf=malloc(rbytes))==0) {
      free(sbuf);
      return(-1);
    }
    for (i=0,sbytes=0; i<sparts; sbytes+=smsg[i]->mxfer_len,i++)
      memcpy(sbuf+sbytes,smsg[i]->mxfer_off,smsg[i]->mxfer_len);
    if ( (j=snd(pid,sbuf,rbuf,sbytes,rbytes))==0)
      for (i=0; i<rparts; rbuf+=rmsg[i]->mxfer_len,i++)
        memcpy(rmsg[i]->mxfer_off,rbuf,rmsg[i]->mxfer_len);
    free(sbuf);
    free(rbuf);
    return(j);
  }
}
