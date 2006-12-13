/*
	Inter Process Communication Initialisation for DAS programs.
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int ipc_init(int flag) {
char fif[12];
	switch (flag) {
		case IPC_QNX: break;
		case IPC_FIFO:
			/* make 2 fifos, one for receive(0) and one for replies */
			sprintf(fif,"%d",getpid());
			if (mkfifo(fif,0)==-1) msg(MSG_FATAL,"can't make fifo %s",fif);
			/* close stdin */
			/* open fif */
			sprintf(fif,"%dR",getpid());
			if (mkfifo(fif,0)==-1) msg(MSG_FATAL,"can't make fifo %s",fif);
			/* open fif */
			/* return fd */
			break;
		case IPC_PIPE:
			break;
	}
	return(flag);
}
