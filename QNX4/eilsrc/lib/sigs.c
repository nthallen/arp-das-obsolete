#include <unistd.h>
#include <signal.h>
#include <sigs.h>

void alarmfunction(int sig_number) {
    signal(SIGALRM,alarmfunction);
    if (breaksignal) alarm(TIMEOUT);
}

void breakfunction(int sig_number) {
    breaksignal = sig_number;
    signal(SIGINT,breakfunction);
    signal(SIGTERM,breakfunction);
    if (sig_number) alarm(TIMEOUT);
    else alarm(0);
}

int breaksignal = 0;
