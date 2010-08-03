#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <fcntl.h>
#include <ctype.h>
#include "nortlib.h"
#include "company.h"
#include "subbusd_int.h"
#include "nl_assert.h"

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;
#define DEVNAME "/dev/" COMPANY "/subbus"

static sbd_request_t sbdrq[SUBBUSD_MAX_REQUESTS];
static sbd_request_t *cur_req;
static unsigned int sbdrq_head = 0, sbdrq_tail = 0;
static int sb_fd;
static struct sigevent ionotify_event;
static char sb_ibuf[SUBBUSD_MAX_REQUEST];
static int sb_ibuf_idx = 0;

// Transmits a request if the currently queued
// request has not been transmitted.
static void process_request(void) {
  int cmdlen, n;
  sbd_request_t *sbr;
  if ( sbdrq_head == sbdrq_tail || cur_req != NULL)
    return;
  nl_assert( sbdrq_head < SUBBUSD_MAX_REQUESTS );
  sbr = &sbdrq[sbdrq_head];
  nl_assert( sbr->status == SBDR_STATUS_QUEUED );
  switch (sbr->type) {
    case SBDR_TYPE_INTERNAL:
      switch (sbr->request[0]) {
	case '\n': // NOP
	case 'B':  // Board Revision
	  break;
	default:
	  nl_error( 4, "Invalid internal request" );
      }
      break;
    case SBDR_TYPE_CLIENT:
      switch (sbr->request[0]) {
	case 'R':
	case 'W':
	  break;
	default:
	  nl_error( 4, "Invalid client request: '%c'", sbr->request[0] );
      }
    default:
      nl_error(4, "Invalid request type" );
  }
  cmdlen = strlen( sbr->request );
  n = write(sb_fd, sbr->request, cmdlen);
  nl_assert( n == cmdlen );
  sbr->status = SBDR_STATUS_SENT;
  cur_req = sbr;
}

static void enqueue_sbreq( int type, int rcvid, char *req ) {
  sbd_request_t *sbr = &sbdrq[sbdrq_tail];
  int i;
  int new_tail = sbdrq_tail+1;
  int old_tail;
  if ( new_tail >= SUBBUSD_MAX_REQUESTS ) new_tail = 0;
  if ( new_tail == sbdrq_head )
    nl_error( 4, "Request queue overflow" );
  for ( i = 0; i < SUBBUSD_MAX_REQUEST && req[i] != '\0'; ++i );
  if ( i >= SUBBUSD_MAX_REQUEST )
    nl_error( 4, "Request exceeds %d characters", SUBBUSD_MAX_REQUEST );
  else ++i; // count the trailing nul
  sbr->type = type;
  sbr->rcvid = rcvid;
  strncpy(sbr->request, req, SUBBUSD_MAX_REQUEST);
  sbr->status = SBDR_STATUS_QUEUED;
  old_tail = sbdrq_tail;
  sbdrq_tail = new_tail;
  if ( sbdrq_head == old_tail ) process_request();
}

static int subbus_io_msg(resmgr_context_t *ctp, io_msg_t *msg,
               RESMGR_OCB_T *ocb) {
  subbusd_msg_t sbdmsg;
  
  MsgRead (ctp->rcvid, &sbdmsg, sizeof (sbdmsg), 0);
  if (sbdmsg.hdr.mgrid != SUBBUSD_MGRID)
    return (ENOSYS);
  enqueue_sbreq( SBDR_TYPE_CLIENT, ctp->rcvid, sbdmsg.request );
  // MsgReply( ctp->rcvid, 0, &my_reply, sizeof(my_reply));
  return (_RESMGR_NOREPLY);
}

static int sb_data_arm(void) {
  int cond;
  cond = ionotify( sb_fd, _NOTIFY_ACTION_POLLARM,
    _NOTIFY_COND_INPUT, &ionotify_event );
  if ( cond == -1 )
    nl_error( 3, "Error from ionotify: %s", strerror(errno));
  return cond;
}

// dequeue_request() sends the response to client
// requests and dequeues the current request.
static void dequeue_request( char *response, int nb ) {
  int rv;

  nl_assert( cur_req != NULL);
  switch( cur_req->type ) {
    case SBDR_TYPE_INTERNAL:
      switch (response[0]) {
	case '0': break;
	case 'V':
	  nl_error( 0, "Version %s", response+1 );
	  break;
	default:
	  nl_error( 4, "Invalid response in dequeue_request" );
      }
      break;
    case SBDR_TYPE_CLIENT:
      rv = MsgReply( cur_req->rcvid, 0, response, nb+1 );
      break;
    default:
      nl_error( 4, "Invalid command type in dequeue_request" );
  }
  cur_req = NULL;
  if ( ++sbdrq_head >= SUBBUSD_MAX_REQUESTS )
    sbdrq_head = 0;
  if (rv == -1)
    nl_error(2, "Error from MsgReply: %s",
      strerror(errno) );
}

void process_interrupt(char *resp, int nb ) {
}

/* process_response() reviews the response in
   the buffer to determine if it is a suitable
   response to the current request. If so, it
   is returned to the requester.
   process_response() is not responsible for
   advancing to the next request, but it is
   responsible for dequeuing the current
   request if it has been completed.
 */
#define RESP_OK 0
#define RESP_UNRECK 1
#define RESP_UNEXP 2
#define RESP_INV 3
#define RESP_INTR 4

void process_response( char *buf, int nb ) {
  int status = RESP_OK;
  char curcmd = '\0';
  nl_assert( nb > 0 );
  if ( cur_req ) {
    curcmd = cur_req->request[0];
    switch ( buf[0] ) {
      case 'R':
      case 'r':
	if ( curcmd != 'R' ) status = RESP_UNEXP;
	else if ( nb != 5 ) status = RESP_INV;
	else {
	  int i;
	  for (i = 1; i < 5; i++) {
	    if ( ! isxdigit(buf[i]))
	      status = RESP_INV;
	  }
	}
	break;
      case 'W':
      case 'w':
	if ( curcmd != 'W' ) status = RESP_UNEXP;
	else if (nb != 1) status = RESP_INV;
	break;
      case 'S':
      case 'C':
	if ( curcmd != buf[0] ) status = RESP_UNEXP;
	else if ( ( buf[1] != '0' && buf[1] != '1' ) || nb != 2 )
	  status = RESP_INV;
	break;
      case 'V':
	if ( curcmd != 'V' ) status = RESP_UNEXP;
	break;
      case '0':
	if ( curcmd != '\n' ) status = RESP_UNEXP;
	break;
      case 'I':
	status = RESP_INTR; break;
      default:
	status = RESP_UNRECK; break;
    }
    nl_error( 1, "Unexpected response: '%s'", buf );
  } else {
    switch ( buf[0] ) {
      case 'I':
	status = RESP_INTR; break;
      default:
      	status = RESP_UNEXP; break;
    }
  }
  switch (status) {
    case RESP_OK:
      dequeue_request(buf, nb);
      break;
    case RESP_INTR:
      process_interrupt(buf, nb);
      break;
    case RESP_UNRECK:
      nl_error( 2, "Unreckognized response: '%s'", buf );
      break;
    case RESP_UNEXP:
      nl_error( 2, "Unexpected response: '%s'", buf );
      break;
    case RESP_INV:
      nl_error( 2, "Invalid response: '%s'", buf );
      break;
    default:
      nl_error( 4, "Invalid status: %d", status );
  }
  // we won't dequeue on error: wait for timeout to handle that
  // that's because we don't know the invalid response was
  // to the current request. It could be noise, or an invalid
  // interrupt response for something.
}

/* sb_read_usb() reads data from the serusb device and
   decides what to do with it. If it satisfies the current
   request, we reply to the caller.
 */
void sb_read_usb(void) {
  do {
    int nb, nbr;

    nbr = SUBBUSD_MAX_REQUEST - sb_ibuf_idx;
    nl_assert(nbr > 0 && nbr < SUBBUSD_MAX_REQUEST);
    nb = read(sb_fd, &sb_ibuf[sb_ibuf_idx], nbr);
    if ( nb < 0 )
      nl_error( 3, "Error on read: %s", strerror(errno));
    nl_assert(nb >= 0 && nb <= nbr);
    // Check to see if we have a complete response
    while ( nb > 0 ) {
      if ( sb_ibuf[sb_ibuf_idx] == '\n' ) {
	sb_ibuf[sb_ibuf_idx] = '\0';
	process_response(sb_ibuf, sb_ibuf_idx);
	if (--nb > 0) {
	  memmove( sb_ibuf, &sb_ibuf[sb_ibuf_idx+1], nb );
	  sb_ibuf_idx = 0;
	  // do not issue any pending request
	  // in order to preserve causality
	} else {
	  sb_ibuf_idx = 0;
	  process_request();
	}
      } else {
	++sb_ibuf_idx;
	--nb;
      }
    }
  } while ( sb_data_arm() | _NOTIFY_COND_INPUT );
}

/* sb_data_ready() is a thin wrapper for sb_read_usb()
   which is invoke from dispatch via pulse_attach().
 */
int sb_data_ready( message_context_t * ctp, int code,
	unsigned flags, void * handle ) {
  sb_read_usb();
  return 0;
}

static void init_serusb(dispatch_t *dpp, int ionotify_pulse) {
  sb_fd = open("/dev/serusb2", O_RDWR | O_NONBLOCK);
  if (sb_fd == -1)
    nl_error(3,"Error opening USB subbus: %s", strerror(errno));
  /* flush anything in the input buffer */
  { int n;
    char tbuf[256];
    do {
      n = read(sb_fd, tbuf, 256);
    } while (n < 256);
  }
  ionotify_event.sigev_notify = SIGEV_PULSE;
  ionotify_event.sigev_code = ionotify_pulse;
  ionotify_event.sigev_priority = getprio(0);
  ionotify_event.sigev_value.sival_int = 0;
  ionotify_event.sigev_coid =
    message_connect(dpp, MSG_FLAG_SIDE_CHANNEL);
  if ( ionotify_event.sigev_coid == -1 )
    nl_error(3, "Could not connect to our channel: %s",
      strerror(errno));

  /* now arm for input */
  sb_read_usb();

}

int main(int argc, char **argv) {
  resmgr_attr_t        resmgr_attr;
  dispatch_t           *dpp;
  dispatch_context_t   *ctp;
  int                  id;
  int                  ionotify_pulse;

  /* initialize dispatch interface */
  if((dpp = dispatch_create()) == NULL) {
      nl_error(3,
	      "%s: Unable to allocate dispatch handle.\n",
	      argv[0]);
      return EXIT_FAILURE;
  }

  /* initialize resource manager attributes */
  memset(&resmgr_attr, 0, sizeof resmgr_attr);
  resmgr_attr.nparts_max = 1;
  resmgr_attr.msg_max_size = 2048;

  /* initialize functions for handling messages */
  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
		   _RESMGR_IO_NFUNCS, &io_funcs);

  io_funcs.msg = subbus_io_msg;

  /* initialize attribute structure used by the device */
  iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

  /* attach our device name */
  id = resmgr_attach(
	  dpp,            /* dispatch handle        */
	  &resmgr_attr,   /* resource manager attrs */
	  DEVNAME,        /* device name            */
	  _FTYPE_ANY,     /* open type              */
	  0,              /* flags                  */
	  &connect_funcs, /* connect routines       */
	  &io_funcs,      /* I/O routines           */
	  &attr);         /* handle                 */
  if (id == -1)
      nl_error( 3, "%s: Unable to attach name.\n", argv[0]);

  /* Setup ionotify pulse handler */
  ionotify_pulse =
    pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, sb_data_ready, NULL);
  init_serusb(dpp, ionotify_pulse);

  /* Setup timer pulse handler */
  // pulse_attach();

  /* Open serusb device */
  /* Enqueue initialization request */

  /* allocate a context structure */
  ctp = dispatch_context_alloc(dpp);

  /* start the resource manager message loop */
  while(1) {
      if((ctp = dispatch_block(ctp)) == NULL) {
	  nl_error( 3, "block error\n");
	  return EXIT_FAILURE;
      }
      dispatch_handler(ctp);
  }
}

