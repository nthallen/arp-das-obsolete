#ifndef DataQueue_H_INCLUDED
#define DataQueue_H_INCLUDED
#include "tm.h"


class tstamp_q {
  public:
    tstamp_q( mfc_t MFCtr, time_t time );
    void unreference();
    tstamp_t TS;
};

// Define a hierarchy here. A dq_descriptor can either hold
// a timestamp or reference data rows in the DQ.
// This works within DG because we don't have readers starting
// and stopping. We have exactly one reader that will go
// through all the data.
enum dqtype { dq_tstamp, dq_data  };
class dq_ref {
  public:
    dq_ref();
    dqtype type;
}

class dq_tstamp_ref : public dqtype {
  public:
    dq_tstamp_ref();
    tstamp_t TS;
};

class dq_data_ref : public dqtype {
  public:
    dq_data_ref();
    mfc_t MFCtr_start, MFCtr_next;
    int row_start, row_next;
    int Qrow;
    int n_rows;
};

/* Semantics of Data_Queue
   Data_Queue.first, .last are indices into row and range from
     [0..total_Qrows)
   .first is where the next row will be read from
   .last is where the next row will be written to
   
   if collection is true, then allocate_rows will throw rather than block
   
   if read_coid >= 0, transmits that cannot be satisfied immediately will
   return a value of 0 and then send a pulse when data is ready.
   if read_coid < 0, such requests will block
*/
class data_queue {
  public:
    data_queue( int collection == 0, int read_coid == -1 );
    int allocate_rows( int n_rows );
    void commit_rows( mfc_t MFCtr, int row_start, int n_rows );
    void commit_tstamp( mfc_t MFCtr, time_t time );
    int transmit_rows( int n_rows );
    
    unsigned char *raw;
    unsigned char **row;
    tm_hdrw_t input_tm_type;
    tm_hdrw_t output_tm_type;
    int pbuf_size; // nbQrow+nbDataHdr (or more)
    int total_size;
    int total_Qrows;
    int nbQrow; // may differ from nbrow if stripping MFCtr & Synch
    int nbDataHdr;
    int first;
    int last;
    int nonblocking;
};


#endif
