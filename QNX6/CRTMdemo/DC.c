#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "tablelib.h"
#include "dbr.h"
#include "globmsg.h"
#include "nortlib.h"
#include "Inet.h"

extern void phinitfunc(void);

static struct {
  struct hdrs_s {
	msg_hdr_type Inet_hdr;
	msg_hdr_type hdr;
  } hdrs;
  union {
	dbr_data_type data;
	tstamp_type tstamp;
	dascmd_type cmd;
  } u;
} __attribute__((packed)) *DC_msg_buf;
dbr_info_type dbr_info;

static int DC_nb_req, DC_nb_buf;

#define DC_HDR_SIZE sizeof(struct hdrs_s)
#define DC_NROWS_SIZE (DC_HDR_SIZE+sizeof(token_type))
#define DC_CMD_SIZE (DC_HDR_SIZE+sizeof(dascmd_type))
#define DC_TSTAMP_SIZE (DC_HDR_SIZE+sizeof(tstamp_type))

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

