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
  nb = read( bfr_fd, buf + bytes_read, bufsize-bytes_read);
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


void reset_DCbuf( void ) {
  DC_nb_req = DC_HDR_SIZE;
  DC_nb_buf = 0;
}

void handle_message( void ) {
  #ifdef PEDANTIC
  if (DC_msg_buf->hdrs.Inet_hdr != INET_HDR_BYTE )
    nl_error( 4, "DC: Bad INET_HDR_BYTE" );
  #endif
  if ( DC_nb_buf == DC_HDR_SIZE ) {
    switch ( DC_msg_buf->hdrs.hdr ) {
      case DCDATA: DC_nb_req = DC_NROWS_SIZE; break;
      case DCDASCMD: DC_nb_req = DC_CMD_SIZE; break;
      case TSTAMP: DC_nb_req = DC_TSTAMP_SIZE; break;
      default:
        nl_error( 3, "DC: 1 Unexpected type code %d",
           DC_msg_buf->hdrs.hdr );
    }
  } else {
    switch ( DC_msg_buf->hdrs.hdr ) {
      case DCDATA:
        if ( DC_nb_buf == DC_NROWS_SIZE ) {
          DC_nb_req += DC_msg_buf->u.data.n_rows * tmi(nbrow);
        } else {
          DC_data( &DC_msg_buf->u.data );
          reset_DCbuf();
        }
        break;
      case DCDASCMD:
        DC_DASCmd( DC_msg_buf->u.cmd.type, DC_msg_buf->u.cmd.val );
        if ( DC_msg_buf->u.cmd.type == DCT_QUIT ) PtExit(0);
        reset_DCbuf();
        break;
      case TSTAMP:
        DC_tstamp( &DC_msg_buf->u.tstamp );
        reset_DCbuf();
        break;
      default:
        nl_error( 4, "DC: 2 Unexpected type code %d",
           DC_msg_buf->hdrs.hdr );
    }
  }
}

PtFdProcF_t DC_fd_handler;
int DC_fd_handler( int fd, void *data, unsigned mode) {
  char *cbuf = (char *)DC_msg_buf;
  for (;;) {
    int toread = DC_nb_req - DC_nb_buf;
    if ( toread == 0 ) handle_message();
    else {
      int nb;
      nb = read( STDIN_FILENO, cbuf + DC_nb_buf, toread );
      if ( nb == 0 ) break;
      if ( nb == -1 ) {
        if (errno == EAGAIN ) break;
        else nl_error( 4, "DC: Error reading from stdin: %s",
          strerror(errno) );
      }
      if ( nb > toread )
        nl_error( 4, "DC: Unexpected overrun from read" );
      DC_nb_buf += nb;
    }
  }
  PtFlush();
  return Pt_CONTINUE;
}

int DC_operate( void ) {
  { int nbytes = sizeof(dbr_info);
    char *buf = (char *)(&dbr_info);
    while ( nbytes > 0 ) {
      int nb = read( STDIN_FILENO, buf, nbytes );
      if ( nb == 0 )
        nl_error( 4, "DC: Unexpected read of 0 bytes: %s",
                strerror(errno) );
      nbytes -= nb;
      buf += nb;
    }
    printf( "DC: nbrow = %d\n", tmi(nbrow) );
  }
  { int cntlflags = fcntl( STDIN_FILENO, F_GETFL );
    if (cntlflags == -1 )
      nl_error( 3, "Error from fcntl: %s", strerror(errno) );
    cntlflags = fcntl( STDIN_FILENO, F_SETFL, cntlflags | O_NONBLOCK );
    if ( cntlflags == -1 )
      nl_error( 3, "Error from fcntl setting NONBLOCK: %s",
         strerror(errno) );
  }
  DC_msg_buf = malloc(MAX_BUF_SIZE+1);
  if ( DC_msg_buf == 0 )
    nl_error( 3, "Out of memory for DC_msg_buf" );
  reset_DCbuf();
  phinitfunc();
  PtAppAddFd( NULL, STDIN_FILENO, Pt_FD_READ, DC_fd_handler, NULL );
  PtMainLoop();
  return 0;
}

