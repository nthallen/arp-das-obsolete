#ifndef DataQueue_H_INCLUDED
#define DataQueue_H_INCLUDED
#include <semaphore.h>
#include "tm.h"
#include "DG_tmr.h"

// I prefer not ot allocate and free these structures routinely, but I'll start that way.
// It makes sense to keep a free list for the basic types: tstamp_q, dq_tstamp_ref and dq_data_ref
// Actually, I guess that can be optimized via definition of new and delete operators.

// Define a hierarchy here. A dq_descriptor can either hold
// a timestamp or reference data rows in the DQ.
// This works within DG because we don't have readers starting
// and stopping. We have exactly one reader that will go
// through all the data.
enum dqtype { dq_tstamp, dq_data  };

class dq_ref {
  public:
    dq_ref(dqtype mytype);
    dq_ref *next(dq_ref *dqr);
    dq_ref *next_dqr;
    dqtype type;
};

class dq_tstamp_ref : public dq_ref {
  public:
    dq_tstamp_ref( mfc_t MFCtr, time_t time );
    tstamp_t TS;
};

class dq_data_ref : public dq_ref {
  public:
    dq_data_ref(mfc_t MFCtr, int mfrow, int Qrow_in, int nrows_in );
    void dq_data_ref::append_rows( int nrows );
    mfc_t MFCtr_start, MFCtr_next;
    int row_start, row_next;
    int Qrow;
    int n_rows;
};

typedef int dq_bool; // Our own boolean type

/* Semantics of Data_Queue
   Data_Queue.first, .last are indices into row and range from
     [0..total_Qrows)
   .first is where the next row will be read from
   .last is where the next row will be written to
   first==last means either full or empty, depending on the value of full.
   
   if collection is true, then allocate_rows will throw rather than block
*/
class data_queue {
  public:
    data_queue( dq_bool collect, int n_Qrows, int n_req );
    void control_thread();
    void read_thread(void *arg);
    void write_thread(void *arg);
    int allocate_rows();
    void commit_rows( mfc_t MFCtr, int mfrow, int n_rows );
    void commit_tstamp( mfc_t MFCtr, time_t time );
    void service_timer();

  private:
    void commit();
    int transmit_rows( int n_rows );
    
    dq_bool collection; // True only for collection
    dq_bool tm_start; // True while running
    dq_bool tm_quit; // non-zero means we are terminating
    dq_bool timed; // True whenever data flow is time-based
    
    void lock();
    void unlock();

    unsigned char *raw;
    unsigned char **row;
    tm_hdrw_t output_tm_type;
    // int pbuf_size; // nbQrow+nbDataHdr (or more)
    // int total_size;
    int total_Qrows;
    int nbQrow; // may differ from nbrow if stripping MFCtr & Synch
    int nbDataHdr;
    int first;
    int last;
    dq_bool full;
    
    dq_ref *first_dqr;
    dq_ref *last_dqr;
    pthread_mutex_t dq_mutex;
    int wr_rows_requested;
    enum rd_block_modes { rb_command, rb_time, rb_data, rb_time_data } rd_block_mode;
    dq_bool rd_blocked;
    sem_t read_sem;
    
    enum wr_block_modes { wb_command, wb_time, wb_data } wr_block_mode;
    dq_bool wr_blocked;
    sem_t write_sem;
    
    DG_tmr *tmr;
};

#endif
