#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "DC.h"
#include "nortlib.h"

unsigned int data_client::next_minor_frame;
unsigned int data_client::minf_row;
unsigned int data_client::majf_row;

data_client::data_client(int bufsize_in, int fast, int non_block) {
  bufsize = bufsize_in;
  bytes_read = 0;
  next_minor_frame = 0;
  majf_row = 0;
  minf_row = 0;
  toread = sizeof(tm_hdr_t);
  buf = new char[bufsize];
  tm_info_ready = false;
  quit = false;
  if ( buf == 0)
    nl_error( 3, "Memory allocation failure in data_client::data_client");
  msg = (tm_msg_t *)buf;
  non_block = non_block ? O_NONBLOCK : 0;
  bfr_fd = open(tm_dev_name( fast ? "TM/DCf" : "TM/DCo" ),
    O_RDONLY | non_block );
  if ( bfr_fd == -1 )
    nl_error( 3, "Unable to contact TMbfr: %d", errno );
}

void data_client::read() {
  int nb;
  nb = ::read( bfr_fd, buf + bytes_read, bufsize-bytes_read);
  if (nb == 0) {
    quit = true;
    return;
  }
  if ( nb == -1 ) {
    if (errno == EAGAIN) return; // must be non-blocking
    else nl_error( 4, "data_client::read error from read(): %s",
      strerror(errno) );
  }
  bytes_read += nb;
  if ( bytes_read >= toread )
    process_message();
}

/** This is the basic operate loop for a simple extraction
 *
 */
void data_client::operate() {
  while ( !quit ) {
    read();
  }
}

void data_client::process_init() {
  // verify tm_info (assuming it was pre-defined)
  // determine output_tm_type
  // define nbQrow and nbDataHdr
  // set tm_info_ready
}

void data_client::process_tstamp() {
  tm_info.t_stmp = msg->body.ts;
}

void data_client::process_message() {
  while ( bytes_read >= sizeof(tm_hdr_t) ) {
    if ( msg->hdr.tm_id != TMHDR_WORD )
      nl_error( 3, "Invalid data from TMbfr" );
    if ( !tm_info_ready ) {
      if ( msg->hdr.tm_type != TMTYPE_INIT )
        nl_error( 3, "Expected TMTYPE_INIT" );
      toread = sizeof(tm_hdr_t)+sizeof(tm_info_t);
      if ( bytes_read >= toread )
        process_init();
    } else {
      switch ( msg->hdr.tm_type ) {
        case TMTYPE_INIT: nl_error( 3, "Unexpected TMTYPE_INIT" ); break;
        case TMTYPE_TSTAMP: process_tstamp(); break;
        case TMTYPE_DATA_T1:
        case TMTYPE_DATA_T2:
        case TMTYPE_DATA_T3:
        case TMTYPE_DATA_T4:
          if ( msg->hdr.tm_type != output_tm_type )
            nl_error(3, "Invalid data type: %04X", msg->hdr.tm_type );
          toread = nbDataHdr + nbQrow * msg->body.data1.n_rows;
          if ( bytes_read >= toread ) process_data();
          break;
        default: nl_error( 3, "Invalid TMTYPE: %04X", msg->hdr.tm_type );
      }
    }
    if ( bytes_read > toread ) {
      memmove(buf, buf+bytes_read, bytes_read - toread);
      bytes_read -= toread;
      toread = sizeof(tm_hdr_t);
    }
  }
}
