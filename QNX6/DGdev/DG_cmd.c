#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "DG_Resmgr.h"
#include "DG_cmd.h"
#include "nl_assert.h"
#include "tm.h"

static DG_cmd *Cmd;
resmgr_connect_funcs_t DG_cmd::connect_funcs;
resmgr_io_funcs_t DG_cmd::io_funcs;
iofunc_attr_t DG_cmd::cmd_attr;

int DG_cmd::execute(char *buf) {
	assert(buf != 0);
	int len = strlen(buf);
	if ( len == 0 ) {
		nl_error( 0, "Zero returned from read" );
		dispatch->ready_to_quit();
		return 1;
	} else {
		if ( len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
		nl_error( 0, "Execute: '%s'", buf );
	}
	return 0;
}

/* return non-zero if a quit command is received */
int DG_cmd::service_pulse( int triggered ) {
  for (;;) {
    int rc;
    char buf[DG_cmd::DG_CMD_BUFSIZE+1];

    if ( triggered ) {
      nl_error( 0, "Triggered:" );
      rc = read( cmd_fd, buf, DG_CMD_BUFSIZE );
      if ( rc < 0 ) nl_error( 2, "Error %d from read", errno );
      else if (execute(buf)) return 1;
    }
    rc = ionotify( cmd_fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_INPUT, &cmd_ev );
    if ( rc < 0 ) nl_error( 3, "Error %d returned from ionotify()", errno );
    if ( rc == 0 ) return 0;
  }
}

DG_cmd::DG_cmd( DG_dispatch *disp ) {
	dispatch = disp;
	dispatch_t *dpp = dispatch->dpp;
	if (Cmd != NULL)
		nl_error(3,"Only one DG_cmd instance allowed");
 
  // This is our write-only command interface
  resmgr_attr_t resmgr_attr;
  memset(&resmgr_attr, 0, sizeof(resmgr_attr));
  resmgr_attr.nparts_max = 1;
  resmgr_attr.msg_max_size = DG_CMD_BUFSIZE+1;

  iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
      _RESMGR_IO_NFUNCS, &io_funcs );
  io_funcs.write = DG_cmd_io_write;
  
  iofunc_attr_init( &DG_cmd::cmd_attr, S_IFNAM | 0222, 0, 0 ); // write-only
  char *wr_devname = tm_dev_name( "DG/cmd" );
  dev_id = resmgr_attach( dpp, &resmgr_attr, wr_devname, _FTYPE_ANY, 0,
  					&DG_cmd::connect_funcs, &DG_cmd::io_funcs, &DG_cmd::cmd_attr );
  if ( dev_id == -1 )
    nl_error( 3, "Unable to attach name %s: errno %d", wr_devname, errno );
 
  
  // This is the read stuff
  char *rd_devname = tm_dev_name( "cmd/DG" );
  cmd_fd = open( rd_devname, O_RDONLY );
  if ( cmd_fd < 0 ) {
    nl_error( 1, "Unable to read from %s", rd_devname );
  } else {
	  int pulse_code =
	    pulse_attach( dpp, MSG_FLAG_ALLOC_PULSE, 0, DG_cmd_pulse_func, NULL );
	  if ( pulse_code < 0 )
	    nl_error(3, "Error %d from pulse_attach", errno );
	  int coid = message_connect( dpp, MSG_FLAG_SIDE_CHANNEL );
	  if ( coid == -1 )
	    nl_error(3, "Error %d from message_connect", errno );
	  cmd_ev.sigev_notify = SIGEV_PULSE;
	  cmd_ev.sigev_coid = coid;
	  cmd_ev.sigev_priority = getprio(0);
	  cmd_ev.sigev_code = pulse_code;
	  if ( service_pulse( 0 ) )
	    nl_error( 0, "Quit received during initialization" );
  }
  Cmd = this;
}

DG_cmd::~DG_cmd() {
  close(cmd_fd);
}

int DG_cmd_pulse_func( message_context_t *ctp, int code,
		unsigned flags, void *handle ) {
  assert(Cmd != 0);
  Cmd->service_pulse(1);
  return 0;
}

int DG_cmd_io_write( resmgr_context_t *ctp,
		 io_write_t *msg, RESMGR_OCB_T *ocb ) {
  int status, msgsize;
  char buf[DG_cmd::DG_CMD_BUFSIZE+1];

  status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, NULL);
  if ( status != EOK )
    return status;

  if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
    return ENOSYS;

  msgsize = msg->i.nbytes;
  if ( msgsize > DG_cmd::DG_CMD_BUFSIZE )
    return E2BIG;

  _IO_SET_WRITE_NBYTES( ctp, msg->i.nbytes );

  resmgr_msgread( ctp, buf, msgsize, sizeof(msg->i) );
  buf[msgsize] = '\0';

  // Handle the message
  Cmd->execute(buf);
  return EOK;
}

