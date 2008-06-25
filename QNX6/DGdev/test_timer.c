/* Timer test
   Create a channel
   Create a connection to the channel
   Define an event as Pulse to connection
   Create a timer using event
   Set the timer
   MsgReceive() waiting for the pulse and print something
*/
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/iomsg.h>
#include "nortlib.h"
#include "tm.h"
#include "DG_cmd.h"
#include "DG_tmr.h"

static int quit_received = 0;

/* return non-zero if a quit command is received */
int DG_cmd::service( int triggered ) {
  for (;;) {
    int rc;
    char buf[BUFSIZE];

    if ( triggered ) {
      nl_error( 0, "Triggered:" );
      rc = read( cmd_fd, buf, BUFSIZE-1 );
      if ( rc < 0 ) nl_error( 2, "Error %d from read", errno );
      else if (rc == 0) {
	nl_error( 0, "Zero returned from read" );
	return 1;
      } else {
	buf[rc] = '\0';
	if (rc > 0 && buf[rc-1] == '\n') buf[rc-1] = '\0';
	nl_error( 0, "Read %d: %s", rc, buf );
      }
    }
    rc = ionotify( cmd_fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_INPUT, &cmd_ev );
    if ( rc < 0 ) nl_error( 3, "Error %d returned from ionotify()", errno );
    if ( rc == 0 ) return 0;
  }
}

int DG_cmd_pulse_func( message_context_t *ctp, int code,
		unsigned flags, void *handle ) {
  DG_cmd *cmd = (DG_cmd *)handle;
  if ( cmd->service(1) ) quit_received = 1;
  return 0;
}

DG_cmd::DG_cmd( dispatch_t *dpp ) {
  int pulse_code =
    pulse_attach( dpp, MSG_FLAG_ALLOC_PULSE, 0, DG_cmd_pulse_func, this );
  if ( pulse_code < 0 )
    nl_error(3, "Error %d from pulse_attach", errno );
  int coid = message_connect( dpp, MSG_FLAG_SIDE_CHANNEL );
  if ( coid == -1 )
    nl_error(3, "Error %d from message_connect", errno );
  char *devname = tm_dev_name( "cmd/DG" );
  cmd_fd = open( devname, O_RDONLY );
  if ( cmd_fd < 0 )
    nl_error( 3, "Unable to read from %s", devname );
  cmd_ev.sigev_notify = SIGEV_PULSE;
  cmd_ev.sigev_coid = coid;
  cmd_ev.sigev_priority = getprio(0);
  cmd_ev.sigev_code = pulse_code;
  if ( service( 0 ) )
    nl_error( 0, "Quit received during initialization" );
}

DG_cmd::~DG_cmd() {
  close(cmd_fd);
}

DG_tmr::DG_tmr( int coid, int pulse_code ) {
  struct sigevent tmr_ev;
  int rc;

  tmr_ev.sigev_notify = SIGEV_PULSE;
  tmr_ev.sigev_coid = coid;
  tmr_ev.sigev_priority = getprio(0);
  tmr_ev.sigev_code = pulse_code;
  rc = timer_create( CLOCK_REALTIME, &tmr_ev, &timerid );
  if ( rc < 0 ) nl_error( 3, "Error creating timer" );
}

DG_tmr::~DG_tmr() {
  // timer_delete?();
  nl_error( 0, "Destructing DG_tmr object" );
}

void DG_tmr::settime( int per_sec, int per_nsec ) {
  struct itimerspec itime;

  itime.it_value.tv_sec = itime.it_interval.tv_sec = per_sec;
  itime.it_value.tv_nsec = itime.it_interval.tv_nsec = per_nsec;
  timer_settime(timerid, 0, &itime, NULL);
}

typedef union {
  struct _pulse pulse;
  /* your other message structures would go here too */
} my_message_t;

static resmgr_connect_funcs_t connect_funcs;
static resmgr_io_funcs_t cmd_io_funcs, data_io_funcs;
static iofunc_attr_t cmd_attr; // Per-name data structure

static int all_closed( void ) {
  return( cmd_attr.count == 0 );
}

static int cmd_io_write( resmgr_context_t *ctp,
		 io_write_t *msg, RESMGR_OCT_T *ocb ) {
  int status, msgsize;
  char buf[CMD_MAX_COMMAND_IN+1];

  status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, NULL);
  if ( status != EOK )
    return status;

  if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
    return ENOSYS;

  msgsize = msg->i.nbytes;
  if ( msgsize > CMD_MAX_COMMAND_IN )
    return E2BIG;

  _IO_SET_WRITE_NBYTES( ctp, msg->i.nbytes );

  resmgr_msgread( ctp, buf, msgsize, sizeof(msg->i) );
  buf[msgsize] = '\0';

  // Parse leading options
  return EOK;
}

int main(int argc, char **argv) {
  dispatch_t *dpp = dispatch_create();
  if ( dpp == NULL )
    nl_error( 3, "Failed to allocate dispatch handle." );
  resmgr_attr_t resmgr_attr;
  memset(&resmgr_attr, 0, sizeof(resmgr_attr));
  resmgr_attr.nparts_max = 1;
  resmgr_attr.msg_max_size = 2048;

  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
      _RESMGR_IO_NFUNCS, &cmd_io_funcs );
  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
      _RESMGR_IO_NFUNCS, &data_io_funcs );
  cmd_io_funcs.write = cmd_io_write;
  // data_io_funcs.write = data_io_write;
  // data_io_funcs.notify = data_io_notify;
  // data_io_funcs.close_ocb = data_close_ocb;
  
  iofunc_attr_init( &cmd_attr, S_IFNAM | 0222, 0, 0 ); // write-only
  char *devname = tm_dev_name( "DG/cmd" )
  int dev_id = resmgr_attach( dpp, &resmgr_attr, devname,
      _FTYPE_ANY, 0, &connect_funcs, &io_funcs, &cmd_attr );
  if ( dev_id == -1 )
    nl_error( 3, "Unable to attach name %s: errno %d", devname, errno );
  dispatch_context_t *ctp = dispatch_context_alloc(dpp);
  if ( ctp == NULL )
    nl_error(3, "dispatch_context_alloc failed: errno %d", errno );

  DG_cmd cmd( dpp );
  DG_tmr tmr( coid, timer_pulse_code);
  tmr.settime( 0, 500000000L );

  while (1) {
    ctp = dispatch_block(ctp);
    if ( ctp == NULL )
      nl_error( 3, "Block error: %d", errno );
    dispatch_handler(ctp);
    if ( quit_received && ctp->resmgr_context.rcvid == 0
	  && all_closed() )
      break;
  }
  
#   for (;;) {
#     rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
#     if (rcvid == 0) { /* we got a pulse */
#       if (msg.pulse.code == timer_pulse_code) {
#         printf("we got a pulse from our timer\n");
#       } else if ( msg.pulse.code == cmd_pulse_code ) {
# 	if (cmd.service(1)) break;
#       } else nl_error( 2, "Unrecognized pulse code: %d", msg.pulse.code );
#     } /* else other messages ... */
  }
  return 0;
}
