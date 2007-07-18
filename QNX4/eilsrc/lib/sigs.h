#define TIMEOUT 1

void alarmfunction(int sig_number);
void breakfunction(int sig_number);
void init_sigs(void);
void block_sigs(void);
void unblock_sigs(void);
extern int breaksignal;
