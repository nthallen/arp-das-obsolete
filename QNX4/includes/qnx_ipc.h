#ifndef _IPC_H_INCLUDED
#define _IPC_H_INCLUDED

#ifdef __QNX__
#include <sys/kernel.h>
#endif
#include "mx.h"


#define MSG_QNX_IPC 0
#define FIFO_QNX_IPC 1

extern int ipc;
extern char ipc_dir[];
extern char ipc_tmp[];
extern int b_fifo_in, b_fifo_out;
extern int relay_pid;
extern int fifo_in;

char *qnx_ipc_tmp(pid_t pid, char suffx, char *name);
int qnx_name_a(int nid, char *name);
int qnx_name_d(int nid, int name_id);
int qnx_name_l(int nid, char *name,unsigned size, unsigned *copies);

int qnx_ipc_init(void);
int qnx_ipc_end(void);
int qnx_snd(int pid, void far *smsg,
	void far *rmsg,
	unsigned snbytes, unsigned rnbytes);
int qnx_sndmx(int pid, unsigned sparts, unsigned rparts,
	struct _mxfer_entry *smsg,
	struct _mxfer_entry *rmsg);
int qnx_recmx(int pid,unsigned parts,struct _mxfer_entry *msgmx);
int qnx_rec(int pid, void far *msg, unsigned nbytes);
int qnx_trig(int proxy);
int qnx_relay(int source, int target, char *msg, int size);
int qnx_proxy_a( int pid, char *data, int nbytes, int priority);
int qnx_proxy_d(int pid);
int qnx_pflgs(long bits, long mask, long *old_bits, long *new_bits);
int qnx_rep(int pid, void far *msg, unsigned nbytes);
int qnx_repmx(pid_t pid, unsigned parts, struct _mxfer_entry *msgmx);
#endif


