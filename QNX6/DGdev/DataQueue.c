/* DataQueue.c */
#include "DataQueue.h"

extern "C" {
  static void dq_write_thread(void *arg);
  static void dq_read_thread(void *arg);
}

/**
 * The global telemetry frame parameter definition structure.
 * Declared in tm.h
 */
tm_info_t tm_info;

/**
 * Base class for dq_data_ref and dq_tstamp_ref
 * These make up part of the control structure of data_queue.
 */
dq_ref::dq_ref(dqtype mytype) {
  next_dqr = 0;
  type = mytype;
}

/**
 * Convenience function to append an item to the end of the linked list and return the new item
 */
dq_ref *dq_ref::next(dq_ref *dqr) {
  next_dqr = dqr;
  return dqr;
}

dq_data_ref::dq_data_ref(mfc_t MFCtr, int mfrow, int Qrow_in, int nrows_in )
      : dq_ref(dq_data) {
  MFCtr_start = MFCtr_next = MFCtr;
  mfrow_start = mfrow_next = mfrow;
  Qrow = Qrow_in;
  n_rows = 0;
  append_rows( nrows_in );
}

void dq_data_ref::append_rows( int nrows ) {
  row_next += nrows;
  MFCtr_next += row_next/tm_info.nrowminf;
  row_next = row_next % tminfo.nrowminf;
}

dq_tstamp_ref::dq_tstamp_ref( mfc_t MFCtr, time_t time ) : dq_ref(dq_tstamp) {
  TS.mfc_num = MFCtr;
  TS.secs = time;
}

/**
  * data_queue base class constructor.
  * Determines the output_tm_type and allocates the queue storage.
  */
data_queue::data_queue( int n_Qrows, int low_water ) {
  total_Qrows = n_Qrows;
  dq_low_water = low_water;
  if ( n_req > n_Qrows )
    nl_error( 3, "wr_rows_requested must be <= n_Qrows" );
  quit = false;
  started = false;
  regulated = false;
  regulation_optional = true;
  autostart = false;

  raw = 0;
  rows = 0;
  first = last = 0;
  full = true;
  bfr_fd = -1;
}

/**
 * General DG initialization. Assumes tm_info structure has been defined.
 * Establishes the connection to the TMbfr, specifying the O_NONBLOCK option for collection.
 * Initializes the queue itself.
 * Creates dispatch queue and registers "DG/cmd" device and initializes timer.
 */
void data_queue::init() {
  // Determine the output_tm_type
  nbQrow = tmi(nbrow);
  if (tm_info.nrowminf > 2) {
    output_tm_type = TMTYPE_DATA_T2;
    nbDataHdr = 10;
  } else if ( tmi(mfc_lsb)==0 && tmi(mfc_msb)==1 ) {
    output_tm_type = TMTYPE_DATA_T3;
    nbQrow -= 4;
    nbDataHdr = 8;
  } else {
    output_tm_type = TMTYPE_DATA_T1;
    nbDataHdr = 6;
  }
  if (nbQrow <= 0) nl_error(3,"nbQrow <= 0");
  int total_size = nbQrow * total_Qrows;
  raw = new unsigned char[total_size];
  if ( ! raw )
    nl_error( 3, "memory allocation failure: raw" );
  rows = new unsigned char[total_Qrows][0];
  if ( ! rows )
    nl_error( 3, "memory allocation failure: rows" );
  int i;
  unsigned char *currow = raw;
  for ( i = 0; i < total_Qrows; i++ ) {
    rows[i] = currow;
    currow += nbQrow;
  }
}

void data_queue::lock() {}
void data_queue::unlock() {}

/**
  no longer a blocking function. Returns the largest number of contiguous rows currently free.
  Caller can decide whether that is adequate.
  */
int data_queue::allocate_rows(unsigned char **rowp) {
  int na;
  lock();
  if ( full ) na = 0
  else if ( first > last ) {
    na = first - last;
  } else na = total_Qrows - last;
  unlock();
  if ( rowp != NULL) *rowp = row[last];
  return na;
}

/**
 *  MFCtr, mfrow are the MFCtr and minor frame row of the first row being committed.
 * Does not signal whoever is reading the queue
 * Assumes DQ is locked and unlocks before exit
 */
void data_queue::commit_rows( mfc_t MFCtr, int mfrow, int nrows ) {
  // we (the writer thread) own the last pointer, so we can read it without a lock,
  // but we must lock before writing
  assert( !full );
  assert( last+nrows <= total_Qrows );
  assert( last_dqr != 0 ); // A timestamp must have been committed before
  lock();
  // We need a new dqr if the last one is a dq_tstamp or my MFCtr,mfrow don't match the 'next'
  // elements in the current dqr
  dq_data_ref *dqdr = 0;
  if ( last_dqr->type == dq_data ) {
    dqdr = (dq_data_ref *)last_dqr;
    if ( MFCtr != dqdr->MFCtr_next || mfrow != dqdr->row_next )
      dqdr = 0;
  }
  if ( dqdr == 0 ) {
    dqdr = new dq_data_ref(MFCtr, mfrow, last, nrows); // or retrieve from the free list?
    last_dqr = last_dqr->next(dqdr);
  } else dqdr->append_rows(nrows);
  last += nrows;
  if ( last == total_Qrows ) last = 0;
  if ( last == first ) full = 1;
  unlock();
}

/**
 * Does not signal whoever is reading the queue
 */
void data_queue::commit_tstamp( mfc_t MFCtr, time_t time ) {
  dq_tstamp_ref *dqt = new dq_tstamp_ref(MFCtr, time);
  lock();
  if ( last_dqr ) last_dqr = last_dqr->next(dqt);
  else last_dqr = dqt;
  unlock();
}
