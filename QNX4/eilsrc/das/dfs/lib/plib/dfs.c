#ifdef __QNX__
#include <sys/psinfo.h>
#else
#define _PPF_PRIORITY_REC
#define _PPF_SIGCATCH
#endif
#include <unistd.h>
#include <errno.h>
#include "dfs.h"
#include "msg.h"
#include "fd.h"

/* main Receive */
int DFS_rec(topology_type top) {
  int i;
  if (IS_BUS(top)) i=fd_read(STDIN_FILENO, (char *)dfs_msg, dfs_msg_size);
  else i=qnx_rec(0, dfs_msg, dfs_msg_size);
  return(i);
}

/* Common initialisations for DG's and DC's */
int DFS_init(topology_type top) {
  /* IPC initialisations */
  switch (top) {
  default:
  case RING:
  case STAR:
    if (qnx_ipc_init() == -1) {
      if (errno==EEXIST) {
	msg(MSG_FAIL,"my PID fifos already exist, exiting with status 99");
	exit(99);
      }
      else msg(MSG_EXIT_ABNORM,"Can't initialise IPC");
    }
    dbr_info.next_tid = 0;
    if (qnx_pflgs(~0,_PPF_PRIORITY_REC | _PPF_SIGCATCH, 0, 0)==-1)
      msg(MSG_FAIL,"Can't set process flags");
    break;
  case BUS:
    dbr_info.next_tid = STDOUT_FILENO;
    fd_block(STDOUT_FILENO);
    fd_block(STDIN_FILENO);
    break;
  }
  init_sigs();
  return 0;
}

