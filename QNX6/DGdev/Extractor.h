#ifndef EXTRACTION_H_INCLUDED
#define EXTRACTION_H_INCLUDED
#include "DataQueue.h"

class extraction : public data_generator {
  public:
    extraction(int nQrows, int n_req);
    void init();
    void ext_thread();
  protected:
    void service_timer();
    void lock();
    void unlock();
    pthread_mutex_t dq_mutex;
    enum ext_block_modes { ext_go, ext_stop, ext_wait } ext_block_mode;
    sem_t ext_sem;
    int missed_pulse;
    int get_data(); // returns non-zero on EOF
    void send_cmd(char *cmd);
    int ext_fd;
}

#endif


