#include <unistd.h>
#include <signal.h>
#include "sigs.h"

int breaksignal = 0;
static sigset_t sigs = 0L;

void alarmfunction(int sig_number) {
  signal(SIGALRM,alarmfunction);
  if (breaksignal) alarm(TIMEOUT);
}

void breakfunction(int sig_number) {
  /* set to ignore */
  if (sig_number<0) {
    breaksignal = 0;
    signal(SIGINT,SIG_IGN);
    signal(SIGTERM,SIG_IGN);
    alarm(0);
    return;
  }
  breaksignal = sig_number;
  signal(SIGINT,breakfunction);
  signal(SIGTERM,breakfunction);
  if (sig_number) alarm(TIMEOUT);
  else {
    alarm(0);
    alarmfunction(0);
  }
}

void init_sigs(void) {
  sigemptyset(&sigs);
  sigaddset(&sigs,SIGINT);
  sigaddset(&sigs,SIGTERM); \
}

void block_sigs(void) {
  sigprocmask(SIG_BLOCK,&sigs,0); alarm(0);
}

void unblock_sigs(void) {
  sigprocmask(SIG_UNBLOCK,&sigs,0);
}

