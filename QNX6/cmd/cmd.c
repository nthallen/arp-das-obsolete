#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "oui.h"
#include "cmd_i.h"
#include "nortlib.h"
#include "nl_assert.h"

int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         rd_io_funcs, wr_io_funcs;
static IOFUNC_ATTR_T             wr_attr;
static resmgr_attr_t             resmgr_attr;
static dispatch_t                *dpp;
static IOFUNC_ATTR_T             *rd_attrs;

// ### kluges for testing
static IOFUNC_ATTR_T *if_foo, *if_bar;
char ci_version[] = "Version zero";

static struct ocb *ocb_calloc (resmgr_context_t *ctp, IOFUNC_ATTR_T *device) {
  ocb_t *ocb = calloc( 1, sizeof(ocb_t) );
  printf("ocb_calloc for node %s\n", device->nodename ? device->nodename : "write" );
  if ( ocb == 0 ) return 0;
  /* Initialize any other elements. Currently all zeros is good. */
  /* Increment count on first command */
  if ( device->first_cmd )
    device->first_cmd->ref_count++;
  ocb->next_command = device->first_cmd;
  return ocb;
}

static void ocb_free (struct ocb *ocb) {
  /* Be sure to remove this from the blocking list:
     Actually, there really is no way it should be on
     the blocking list. */
  printf("ocb_free for node %s\n", ocb->hdr.attr->nodename );
  assert( ocb->rcvid == 0 );
  assert( ocb->next_ocb == 0 );
  if ( ocb->next_command ) {
    assert( ocb->next_command->ref_count > 0 );
    ocb->next_command->ref_count--;
  }
  free( ocb );
}

static iofunc_funcs_t ocb_funcs = { /* our ocb allocating & freeing functions */
    _IOFUNC_NFUNCS,
    ocb_calloc,
    ocb_free
};

/* the mount structure, we have only one so we statically declare it */
iofunc_mount_t mountpoint = { 0, 0, 0, 0, &ocb_funcs };

IOFUNC_ATTR_T *cmdsrvr_setup_rdr( char *node ) {
  ioattr_t *rd_attr = new_memory(sizeof(ioattr_t));
  char nodename[80];
  int id;

  /* initialize attribute structure used by the device */
  iofunc_attr_init((iofunc_attr_t *)rd_attr, S_IFNAM | 0444, 0, 0);
  rd_attr->attr.nbytes = 0;
  rd_attr->attr.mount = &mountpoint;
  rd_attr->nodename = nl_strdup( node );
  
  /* Check Experiment variable for sanity: \w[\w.]* */
  /* Build device name */
  snprintf( nodename, 80, "/dev/huarp/%s/cmd/%s", "Exp", node );
  
  rd_attr->first_cmd = rd_attr->last_cmd = new_command();
  rd_attr->next = rd_attrs;
  rd_attrs = rd_attr;

  /* attach our device name */
  id = resmgr_attach(dpp,            /* dispatch handle        */
		     &resmgr_attr,   /* resource manager attrs */
		     nodename,       /* device name            */
		     _FTYPE_ANY,     /* open type              */
		     0,              /* flags                  */
		     &connect_funcs, /* connect routines       */
		     &rd_io_funcs,   /* I/O routines           */
		     rd_attr);       /* handle                 */
  if(id == -1) {
    nl_error( 3, "Unable to attach name: '%s'", nodename );
  }
  return rd_attr;
}

main(int argc, char **argv) {
    /* declare variables we'll be using */
    int use_threads = 0;
    int                  id;

    //oui_init_options( argc, argv );

    /* initialize dispatch interface */
    if((dpp = dispatch_create()) == NULL) {
        fprintf(stderr, "%s: Unable to allocate dispatch handle.\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /* initialize resource manager attributes. */
    /* planning to share this struct between rd and wr */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    // resmgr_attr.nparts_max = 0;
    // resmgr_attr.msg_max_size = 0;

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
                     _RESMGR_IO_NFUNCS, &rd_io_funcs);
    rd_io_funcs.read = io_read;
    /* Will want to handle _IO_NOTIFY at least */
    // rd_io_funcs.notify = io_notify;

    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
                     _RESMGR_IO_NFUNCS, &wr_io_funcs);
    wr_io_funcs.write = io_write;

    /* initialize attribute structure used by the device */
    iofunc_attr_init((iofunc_attr_t *)&wr_attr, S_IFNAM | 0664, 0, 0);
    wr_attr.attr.nbytes = 0;
    wr_attr.attr.mount = &mountpoint;
    wr_attr.nodename = nl_strdup("writer");

    /* Check Experiment variable for sanity: \w[\w.]* */
    /* Build device name */
    /* attach our device name */
    id = resmgr_attach(dpp,            /* dispatch handle        */
                       &resmgr_attr,   /* resource manager attrs */
                       "/dev/huarp/Exp/cmdw",  /* device name            */
                       _FTYPE_ANY,     /* open type              */
                       0,              /* flags                  */
                       &connect_funcs, /* connect routines       */
                       &wr_io_funcs,   /* I/O routines           */
                       &wr_attr);      /* handle                 */
    if(id == -1) {
        fprintf(stderr, "%s: Unable to attach name.\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    if_foo = cmdsrvr_setup_rdr( "foo" );
    if_bar = cmdsrvr_setup_rdr( "bar" );

    if ( use_threads ) {
      thread_pool_attr_t   pool_attr;
      thread_pool_t        *tpp;

      /* initialize thread pool attributes */
      memset(&pool_attr, 0, sizeof pool_attr);
      pool_attr.handle = dpp;
      pool_attr.context_alloc = dispatch_context_alloc;
      pool_attr.block_func = dispatch_block;
      pool_attr.handler_func = dispatch_handler;
      pool_attr.context_free = dispatch_context_free;
      pool_attr.lo_water = 2;
      pool_attr.hi_water = 4;
      pool_attr.increment = 1;
      pool_attr.maximum = 50;     /* allocate a thread pool handle */
      if((tpp = thread_pool_create(&pool_attr,
                                   POOL_FLAG_EXIT_SELF)) == NULL) {
          fprintf(stderr, "%s: Unable to initialize thread pool.\n",
                  argv[0]);
          return EXIT_FAILURE;
      }     /* start the threads, will not return */
      thread_pool_start(tpp);
    } else {
      int running = 2;
      dispatch_context_t   *ctp;
      ctp = dispatch_context_alloc(dpp);
      printf( "\nStarting:\n" );
      while ( running ) {
	if ((ctp = dispatch_block(ctp)) == NULL) {
	  fprintf(stderr, "block error\n" );
	  return EXIT_FAILURE;
	}
	// printf( "  type = %d,%d  attr.count = %d\n",
	//   ctp->resmgr_context.msg->type,
	//   ctp->resmgr_context.msg->connect.subtype, attr.count );
	dispatch_handler(ctp);
	// Need a more sophisticated termination condition here
	// if ( running > 1 && attr.count > 0 ) running = 1;
	// else if ( running == 1 && attr.count == 0 ) running = 0;
      }
      printf( "Terminating\n" );
    }
}

// This is where commands are recieved. We'll require that only one
// command be received per write. That prevents doing something like
//   cat commandlist.txt > /dev/huarp/Exp/cmd/server
// but there's no particular reason why we'd want to do that. In
// actual use, commands come in one at a time. Also, it's easy enough
// to do:
//  cat commandlist.txt | while read line; do
//    echo $line >/dev/huarp/Exp/cmd/server; done
int io_write( resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb ) {
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
  // No spaces, colons or right brackets allowed in mnemonics 
  { char *mnemonic = "--";
    int quiet = 0;
    int testing = 0;
    char *s = buf;

    if ( *s == '[' ) {
      s++;
      if ( isgraph(*s) && *s != ':' && *s != ']' ) {
	mnemonic = s++;
	while ( isgraph(*s) && *s != ':' && *s != ']' )
	  s++;
      }
      if ( !isgraph(*s) ) {
	nl_error( 2, "Invalid mnemonic string" );
	return EINVAL;
      }
      if ( *s == ':' ) {
	int end_of_opts = 0;
	char *ver;

	*s++ = '\0'; // terminate the mnemonic
	// and then handle the options
	while (!end_of_opts) {
	  switch (*s) {
	    case 'T': testing = 1; s++; break;
	    case 'Q': quiet = 1; s++; break;
	    case 'V': // handle version command
	      ver = ++s;
	      while ( *s != ']' && *s != '\0' ) s++;
	      if ( *s == '\0' ) {
		nl_error( 2, "Unterminated version string" );
		return EINVAL;
	      }
	      *s = '\0';
	      if ( strcmp( ver, ci_version ) == 0 )
		return EOK;
	      nl_error( 2, "Command Versions don't match" );
	      return EINVAL;
	    case ']': end_of_opts = 1; break;
	    default:
	      nl_error( 2, "Invalid option" );
	      return EINVAL;
	  }
	}
      }
      // blank out trailing ']' in case it's the end of the mnemonic
      *s++ = '\0';
    }
    { char *cmd = s;
      int len = 0;
      int rv;

      // Now s points to a command we want to parse.
      // Make sure it's kosher
      while ( *s ) {
	if ( ! isprint(*s) && *s != '\n' ) {
	  nl_error( 2, "Invalid character in command" );
	  return EINVAL;
	}
	len++;
	s++;
      }
      if ( len > 0 && cmd[len-1] == '\n' ) len--;
      nl_error( quiet ? -2 : 0, "%s: %*.*s", 
	mnemonic, len, len, cmd );
      cmd_init();
      rv = cmd_batch( cmd, testing );
      ocb->hdr.attr->attr.flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
      switch ( CMDREP_TYPE(rv) ) {
	case 0: return EOK;
	case 1: return ENOENT;
	case 2: return EINVAL;
	default: return EIO;
      }
    }
  }
}

static void read_reply( RESMGR_OCB_T *ocb ) {
  int nb = ocb->nbytes_requested;
  command_out_t *cmd = ocb->next_command;
  int cmdbytes = cmd->cmdlen - ocb->hdr.offset;
  int bytes_returned = nb > cmdbytes ? cmdbytes : nb;

  assert(cmd->ref_count > 0);
  assert(cmdbytes >= 0);
  MsgReply( ocb->rcvid, bytes_returned,
    cmd->command + ocb->hdr.offset, bytes_returned );
  ocb->rcvid = 0;
  if ( bytes_returned < cmdbytes ) {
    ocb->hdr.offset += bytes_returned;
  } else {
    IOFUNC_ATTR_T *handle = ocb->hdr.attr;
    ocb->hdr.offset = 0;
    cmd->ref_count--;
    ocb->next_command = cmd->next;
    ocb->next_command->ref_count++;
    if ( handle->first_cmd->ref_count == 0 &&
	 handle->first_cmd->next != 0 ) {
      handle->first_cmd = free_command( handle->first_cmd );
    }
  }
}

int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb) {
  int status, nonblock = 0;
  IOFUNC_ATTR_T *handle = ocb->hdr.attr;

  if ((status = iofunc_read_verify( ctp, msg,
		     (iofunc_ocb_t *)ocb, NULL)) != EOK)
    return (status);
      
  if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
    return (ENOSYS);

  ocb->rcvid = ctp->rcvid;
  ocb->nbytes_requested = msg->i.nbytes;
  if ( ocb->next_command->cmdlen > ocb->hdr.offset ) {
    // we've got something to send now
    read_reply( ocb );
  } else if (nonblock) {
    ocb->rcvid = 0;
    return EAGAIN;
  } else {
    // Nothing at the moment.
    ocb->next_ocb = handle->blocked;
    handle->blocked = ocb;
  }
  return _RESMGR_NOREPLY;
}

static command_out_t *free_commands;

command_out_t *new_command(void) {
  command_out_t *cmd;
  if ( free_commands ) {
    cmd = free_commands;
    free_commands = cmd->next;
  } else {
    cmd = new_memory(sizeof(command_out_t));
  }
  cmd->next = NULL;
  cmd->ref_count = 0;
  cmd->command[0] = '\0';
  cmd->cmdlen = 0;
  return cmd;
}

// Returns the next command so it's easy to free the
// first command in a list:
// list = free_command( list );
command_out_t *free_command( command_out_t *cmd ) {
  command_out_t *nxt;
  assert( cmd != NULL );
  assert( cmd->ref_count == 0 );
  nxt = cmd->next;
  cmd->next = free_commands;
  free_commands = cmd;
  return nxt;
}

void cmdsrvr_turf( IOFUNC_ATTR_T *handle, char *format, ... ) {
  va_list arglist;
  command_out_t *cmd;
  int nb;

  assert( handle != NULL );
  cmd = handle->last_cmd;
  va_start( arglist, format );
  nb = vsnprintf( cmd->command, CMD_MAX_COMMAND_OUT, format, arglist );
  va_end( arglist );
  if ( nb >= CMD_MAX_COMMAND_OUT ) {
    nl_error( 2, "Output buffer overflow to node %s", handle->nodename );
    cmd->command[0] = '\0';
  } else {
    cmd->cmdlen = nb;
    cmd->next = handle->last_cmd = new_command();
    // Now run the queue
    while ( handle->blocked ) {
      IOFUNC_OCB_T *ocb = handle->blocked;
      handle->blocked = ocb->next_ocb;
      ocb->next_ocb = NULL;
      assert(ocb->hdr.offset == 0);
      read_reply(ocb);
    }
  }
}

// This will be generated by cmdgen. For this test, just
// decide between foo and bar
int cmd_batch( char *cmd, int test ) {
  if ( test ) return 0;
  if ( *cmd == 'f' ) cmdsrvr_turf( if_foo, "%s", cmd );
  else cmdsrvr_turf( if_bar, "%s", cmd );
  return 0;
}
