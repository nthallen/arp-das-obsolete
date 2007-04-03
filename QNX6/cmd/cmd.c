#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include "oui.h"
#include "cmd_i.h"
#include "nortlib.h"

int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         rd_io_funcs, wr_io_funcs;
static IOFUNC_ATTR_T             wr_attr;
static resmgr_attr_t             resmgr_attr;
static dispatch_t                *dpp;

static struct ocb *ocb_calloc (resmgr_context_t *ctp, IOFUNC_ATTR_T *device) {
  ocb_t *ocb = calloc( 1, sizeof(ocb_t) );
  if ( ocb == 0 ) return 0;
  /* Initialize any other elements. Currently all zeros is good. */
  /* Increment count on first command */
  if ( device->commands )
    device->commands->ref_count++;
  ocb->next_command = device->commands;
  return ocb;
}

static void ocb_free (struct ocb *ocb) {
  /* Be sure to remove this from the blocking list:
     Actually, there really is no way it should be on
     the blocking list. */
  if ( ocb->rcvid )
    nl_error( 4, "hold_index non-zero in ocb_free" );
  if ( ocb->next_ocb )
    nl_error( 4, "next_ocb non-zero in ocb_free" );
  if ( ocb->next_command ) {
    if ( ocb->next_command->ref_count <= 0 )
      nl_error( 4, "ref_count <= 0 in ocb_free" );
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

IOFUNC_ATTR_T *setup_rdr( char *node ) {
  ioattr_t *rd_attr = new_memory(sizeof(ioattr_t));
  char nodename[80];
  int id;

  /* initialize attribute structure used by the device */
  iofunc_attr_init((iofunc_attr_t *)rd_attr, S_IFNAM | 0444, 0, 0);
  rd_attr->attr.nbytes = 0;
  rd_attr->attr.mount = &mountpoint;
  
  /* Check Experiment variable for sanity: \w[\w.]* */
  /* Build device name */
  snprintf( nodename, 80, "/dev/huarp/%s/cmd/%s", "Exp", node );
  
  rd_attr->commands = new_command();

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
    // rd_io_funcs.read = io_read;
    /* Will want to handle _IO_NOTIFY at least */
    // rd_io_funcs.notify = io_notify;

    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, 
                     _RESMGR_IO_NFUNCS, &wr_io_funcs);
    wr_io_funcs.write = io_write;

    /* initialize attribute structure used by the device */
    iofunc_attr_init((iofunc_attr_t *)&wr_attr, S_IFNAM | 0664, 0, 0);
    wr_attr.attr.nbytes = 0;
    wr_attr.attr.mount = &mountpoint;

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
    
    setup_rdr( "foo" );
    setup_rdr( "bar" );

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

int io_write( resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb ) {
  int status, msgsize;
  #define LGR_BUF_SIZE 80
  char buf[LGR_BUF_SIZE+1];

  status = iofunc_write_verify(ctp, msg, (iofunc_ocb_t *)ocb, NULL);
  if ( status != EOK )
    return status;

  if ((msg->i.xtype &_IO_XTYPE_MASK) != _IO_XTYPE_NONE )
    return ENOSYS;

  _IO_SET_WRITE_NBYTES( ctp, msg->i.nbytes );

  /* My strategy for the moment will be to only write the first LGR_BUF_SIZE
     characters. Later, I will loop somehow */
  msgsize = msg->i.nbytes;
  if ( msgsize > LGR_BUF_SIZE ) msgsize = LGR_BUF_SIZE;
  resmgr_msgread( ctp, buf, msgsize, sizeof(msg->i) );
  buf[msgsize] = '\0';
  if ( msgsize > 0 && buf[msgsize-1] == '\n' )
    buf[msgsize-1] = '\0';
  printf("lgr: '%s'\n", buf );

  if ( msg->i.nbytes > 0)
    ocb->hdr.attr->attr.flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;
  return _RESMGR_NPARTS(0);
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
  return cmd;
}

// Returns the next command so it's easy to free the
// first command in a list:
// list = free_command( list );
command_out_t *free_command( command_out_t *cmd ) {
  command_out_t *nxt;
  if ( cmd == NULL )
    nl_error( 4, "NULL cmd in free_command" );
  if ( cmd->ref_count )
    nl_error( 4, "Non-zero ref_count in free_command" );
  nxt = cmd->next;
  cmd->next = free_commands;
  free_commands = cmd;
  return nxt;
}
