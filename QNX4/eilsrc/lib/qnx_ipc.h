#ifndef _IPC_H_INCLUDED
#define _IPC_H_INCLUDED

#ifdef __QNX__
#include <sys/kernel.h>
#else
#include "mx.h"
#endif

#define MSG_QNX_IPC 0
#define FIFO_QNX_IPC 1

extern int ipc;
extern char ipc_dir[];
extern int fifo_in, fifo_out, fifo_r;

int qnx_name_attach(int nid, char *name);
int qnx_name_detach(int nid, int name_id, char *name);
int qnx_name_locate(int nid, char *name,unsigned size, unsigned *copies);

int qnx_ipc_init(void);
int snd(int pid, void __far *smsg,
	void __far *rmsg,
	unsigned snbytes, unsigned rnbytes);
int qnx_sndmx(int pid, unsigned sparts, unsigned rparts,
	struct _mxfer_entry __far *smsg,
	struct _mxfer_entry __far *rmsg);
int qnx_recmx(int pid,unsigned parts,struct _mxfer_entry __far *msgmx);
int qnx_rec(int pid, void __far *msg, unsigned nbytes);
int trig(int proxy, char *data, int nbytes);
int proxy_attach( int pid, char *data, int nbytes, int priority);
int proxy_detach(int pid);
int qnx_pflags(long bits, long mask, long *old_bits, long *new_bits);
int qnx_rep(int pid, void __far *msg, unsigned nbytes);
int qnx_repmx(pid_t pid, unsigned parts, struct _mxfer_entry __far *msgmx);
#endif


