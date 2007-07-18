#include "qnx_ipc.h"

int qnx_snd(pid_t pid, void far *smsg,void far *rmsg, 
	    unsigned snbytes, unsigned rnbytes) {
  switch (ipc) {
  default:
#ifdef __QNX__
  case MSG_QNX_IPC: return(Send(pid, smsg, rmsg, snbytes, rnbytes));
#endif
  case FIFO_QNX_IPC:
    {
      struct _mxfer_entry s;
      struct _mxfer_entry r;
      setmx(&s,smsg,snbytes);
      setmx(&r,rmsg,rnbytes);
      return(qnx_sndmx(pid,1,1,&s,&r));
    }
  }
}
