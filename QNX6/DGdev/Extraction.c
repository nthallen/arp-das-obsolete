#include "Extraction.h"

extern "C" {
  static void *ext_thread_wrapper(void *handle);
}

static void *ext_thread_wrapper(void *handle) {
  extraction *ext = (extraction *)handle;
  ext->ext_thread();
  return (void *)0;
}

extraction::extraction(int nQrows, int n_req ) : data_queue(0,nQrows,n_req) {
  regulation_optional = 1;
  
  pthread_mutexattr_t mut_attr;
  int rc = pthread_mutexattr_init(&attr);
  if ( rc != EOK ) nl_error( 3, "pthread_mutexattr_init returnd %d", rc );
  rc = pthread_mutex_init(&dq_mutex);
  if ( rc != EOK ) nl_error( 3, "pthread_mutex_init returned %d", rc );
  ext_block_mode = ext_go;
  if ( sem_init(&ext_sem, 0, 0) )
    nl_error( 4, "sem_init returned %d", errno );
  pthread_create(NULL, NULL, ext_thread_wrapper, this);
  missed_pulse = 0;
  ext_fd = -1;
}

void extraction::lock() {
  int rc = pthread_mutex_lock(&dq_mutex);
  if (rc != EOK) nl_error( 4, "pthread_mutex_lock returned %d", rc );
}

void extraction::unlock() {
  int rc = pthread_mutex_unlock(&dq_mutex);
  if (rc != EOK) nl_error( 4, "pthread_mutex_unlock returned %d", rc );
}

void extraction::service_timer() {
  lock();
  if ( ext_block_mode == ext_wait ) {
    ext_block_mode = ext_go;
    unlock();
    sem_post(&ext_sem);
  } else {
    ++missed_pulse;
    unlock();
  }
}

void extraction::ext_thread() {
  for (;;) {
    lock();
    if ( quit ) break;
    if ( ! started ) {
      ext_block_mode = ext_stop;
      unlock();
      sem_wait(&ext_sem);
    } else if ( regulated ) {
      // timed loop
      for (;;) {
        ext_block_mode = tm_wait;
        unlock();
        sem_wait(&ext_sem);
        lock();
        int breakout = !started || !regulated;
        unlock();
        if (breakout) break;
        transmit_data(1); // only one row. May signal stop
        if (queue_low() && get_data() && queue_empty())
          send_cmd("DGstop\n");
        lock();
      }
    } else {
      // untimed loop
      for (;;) {
        int breakout = !started || regulated;
        unlock();
        if (breakout) break;
        get_data();
        transmit_data(0); // everything. Include checks for time limits?
        lock();
      }
    }
  }
  send_cmd("DGextdone\n");
}

void extraction::send_cmd(char *cmd) {
  if ( ext_fd < 0 ) {
    ext_fd = open(tm_dev_name("DG/cmd"),O_RDONLY);
    if (ext_fd < 0) nl_error(4, "Unable to open DG/cmd: %d", errno);
  }
  write(ext_fd, cmd, strlen(cmd)+1);
}



