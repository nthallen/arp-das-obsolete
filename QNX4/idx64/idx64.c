#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <sys/proxy.h>
#include "nortlib.h"
#include "oui.h"
#include "collect.h"
#include "cltsrvr.h"
#include "intserv.h"
#include "subbus.h"
#include "idx64.h"
#include "idx64int.h"

#ifdef DOCUMENTATION
  Overview of the structure of this application:
  
                         main
			               |
						   v
                        operate
                           |
		 +-----------------+---------------+
		 |                 |               |
		 v                 |               |
    drive_command          |               |   
	     |                 |               |
		 v                 v               v
	queue_request--> service_board     scan_proxy
                           |               |
						   v	           v
	                  execute_cmd <-- service_scan

  There are several other support routines and initializations,
  but these are the main operating procedures.
  
  operate() preforms the main Receive() loop servicing both 
  commands and interrupts, which come in as proxies. Also when 
  scanning, the scan steps are timed by proxies from TM 
  collection.
  
  Commands are passed to to drive_command() which immediately
  executes those that don't touch the hardware and passes the 
  rest on to queue_request().
  
  queue_request() does just that, sets a flag requesting service 
  and then calls service_board() to determine if the channel is 
  ready to service the request.
  
  service_board() is the interrupt service routine. It is passed 
  the index of the board which produced the interrupt. It reads 
  the board's status register and compares that to the request 
  word to determine if any channels need attention. If so, the 
  channel is passed on to execute_cmd().
  
  execute_cmd() is what actually processes the commands which 
  address the hardware. It translates online/offline/altline
  commands to absolute positions and then translates absolute
  positions to relative drives. If the command is a scan, 
  appropriate flags are set and no further action is taken. For 
  drives, the hardware is programmed accordingly and the command 
  is dequeued. If no more commands are enqueued, the request flag 
  is cleared indicating that this channel does not require 
  service when the drive is completed.
  
  scan_proxy() services the TM proxy which drives the scan rate.
  If surveys all of the boards for scans in progress and calls 
  service_scan() for each scan ready for action.
  
  service_scan() issues each drive command directly. When the 
  scan is completed, it dequeues the scan command and calls 
  execute_cmd() to handle the next command in the queue.
#endif

/* Global Variables */
idx64_def idx_defs[ MAX_IDXRS ] = {
  0xA00, "Idx64_0",
  0xA40, "Idx64_1",
  0xA80, "Idx64_2",
  0xAC0, "Idx64_3"
};
idx64_bd *boards[ MAX_IDXRS ];
/* If every channel requires 3 status bits (maximum possible), we
   can fit 16/3 = 5 channel's status in a 16-bit word. Max
   number of channels is MAX_IDXRS*MAX_IDXR_CHANS. Add 4 and
   divide by 5 to get the max number of 16-bit words required
   to hold all the status bits.
*/
#define MAX_WORDS ((MAX_IDXRS*MAX_IDXR_CHANS+4)/5)
unsigned short tm_ptrs[ MAX_WORDS ];
send_id tm_data;
char *idx64_cfg_string;
int idx64_region = 0x40;

#define N_PROXIES  (MAX_IDXRS + 2)
pid_t proxies[ N_PROXIES];
#define CC_PROXY_ID 0
#define SCAN_PROXY_ID 1
#define BD_0_PROXY 2

/* Channel_init() initializes the channel's data structure,
   zeros the hardware's counters and performs default
   configuration.

   Channel_init() is distinguished from config_channels()
   which performs additional configuration of the channels
   based on command-line input.
*/
static void Channel_init( chandef *chan, unsigned short base ) {
  chan->base_addr = base;
  chan->tm_ptr = NULL;
  chan->scan_bit = 0;
  chan->on_bit = 0;
  chan->supp_bit = 0;
  chan->online = 0;
  chan->online_delta = 0;
  chan->offline_delta = 0;
  chan->altline_delta = 0;
  chan->first = NULL;
  chan->last = NULL;
  sbwr( base+6, DFLT_CHAN_CFG );
  sbwr( base+4, 0 ); /* set position to 0 */
  sbwr( base+2, 0 ); /* drive out 0 */
}

/* init_boards will check for the presence of each possible 
  indexer board. If present, it will allocate a structure for
  the board and request a proxy for the board from intserv.
  If intserv doesn't allow us, it is probably because the board
  is already owned by someone else, a pretty good fatal error.
*/
static void init_boards( void ) {
  int i, j;
  idx64_bd *bd;
  idx64_def *bddef;

  for ( i = 0; i < MAX_IDXRS; i++ ) {
	unsigned short val;

	/* if present, allocate structure, get proxy */
	bddef = &idx_defs[i];
	if ( read_ack( 0, bddef->card_base, &val ) != 0 ) {
	  nl_error( -2, "Board %s present", bddef->cardID );
	  bd = boards[i] = new_memory( sizeof( idx64_bd ) );
	  proxies[ BD_0_PROXY + i ] = bd->proxy =
		qnx_proxy_attach( 0, NULL, 0, -1 );
	  if ( bd->proxy == -1 )
		nl_error( 3, "Error attaching proxy for %s", 
						bddef->cardID );
	  IntSrv_Int_attach( bddef->cardID,	bddef->card_base, idx64_region,
						  bd->proxy );
	  bd->request = bd->scans = 0;
	  for ( j = 0; j < MAX_IDXR_CHANS; j++ )
		Channel_init( &bd->chans[j], bddef->card_base + 8 * ( j + 1 ) );
	} else nl_error( -2, "Board %s not present", bddef->cardID );
  }
}

/* idx_cfg_num invokes strtoul() to read a number, verify that a
   number was in fact read (pointer updated) and updates the 
   pointer. idx_cfg_num dies fatally if there was no number
*/
static unsigned short idx_cfg_num( char **s, int base ) {
  char *t;
  unsigned short val;
  
  t = *s;
  val = strtoul( t, s, base );
  if ( t == *s )
	nl_error( 3, "Syntax error in configuration string" );
  return val;
}

/*
    [cfg code][,n_bits][:[cfg code][,n_bits] ...]
	no spaces, default cfg code is C00 (hex). default n_bits is 0
	Later may add ability to read the configuration from a file, 
	but that's a low priority.
*/
static void config_channels( char *s ) {
  int chan = 0;
  unsigned short code, bits;
  int wdno, bitno;
  int bdno, chno;
  idx64_bd *bd;
  chandef *ch;
  
  wdno = bitno = 0;
  while ( *s != '\0' ) {
	if ( *s == ':' ) {
	  s++; /* empty def */
	  chan++;
	} else {
	  /* non-empty definition */
	  if ( *s != ',' ) code = idx_cfg_num( &s, 16 );
	  else code = DFLT_CHAN_CFG;
	  if ( *s == ',' ) {
		s++;
		bits = idx_cfg_num( &s, 10 );
		if ( bits > 3 )
		  nl_error( 3, "bits value greater than 3" );
	  } else bits = 0;
	  if ( bitno + bits > 16 ) {
		wdno++;
		bitno = 0;
	  }
	  bdno = chan / MAX_IDXR_CHANS;
	  chno = chan % MAX_IDXR_CHANS;
	  if ( bdno > MAX_IDXRS )
		nl_error( 3, "Too many channel configurations" );
	  bd = boards[ bdno ];
	  if ( bd == 0 ) {
		nl_error( 1, "Configuration specified for non-existant channel" );
		bitno += bits;
	  } else {
		ch = &bd->chans[ chno ];
		if ( bits != 0 ) {
		  ch->tm_ptr = &tm_ptrs[ wdno ];
		  if ( bits & 1 )
			ch->scan_bit = ( 1 << bitno++ );
		  if ( bits >= 2 ) {
			ch->on_bit = ( 1 << bitno++ );
			ch->supp_bit = ( 1 << bitno++ );
		  }
		}
		if ( code != DFLT_CHAN_CFG )
		  sbwr( ch->base_addr + 6, code );
	  }
	}
  }
}

static void tm_status_set( unsigned short *ptr,
				unsigned short mask, unsigned short value ) {
  if ( ptr != 0 ) {
	*ptr = ( *ptr & ~mask ) | ( value & mask );
	Col_send( tm_data );
  }
}

static void dequeue( chandef *ch ) {
  ixcmdl *im;

  if ( ch != 0 && ch->first != 0 ) {
	im = ch->first;
	ch->first = im->next;
	if ( ch->first == 0 ) ch->last = NULL;
	free( im );
  }
}

/* scan_setup() is used to start and stop scans.
   If start != 0, it sets the scans bit in the bd def and keeps track
   of the number of scans currently active. When the number of
   scans goes up from zero to 1, the scan proxy is requested
   from collection. If that fails, scan_setup will not init
   and will return non-zero.

   If start==0, this is a cleanup command. The scans bit is 
   cleared (if set), and if the global number of scans drops to 
   zero, the scan proxy is reset.
*/ 
static int scan_setup( idx64_bd *bd, unsigned short chno, int start ) {
  static int n_scans = 0;

  if ( start != 0 ) {
	if ( ( bd->scans & (1<<chno) ) == 0 ) {
	  if ( n_scans == 0 ) {
		int resp;

		resp = set_response( 1 );
		proxies[ SCAN_PROXY_ID ] =
		  Col_set_proxy( INDEXER_PROXY_ID, 0 );
		set_response( resp );
		if ( proxies[ SCAN_PROXY_ID ] == -1 )
		  return -1;
	  }
	  n_scans++;
	  bd->scans |= (1<<chno);
	}
  } else {
	if ( ( bd->scans & (1<<chno) ) != 0 ) {
	  assert( n_scans > 0 );
	  if ( --n_scans == 0 ) {
		int resp;

		resp = set_response( 1 );
		Col_reset_proxy( INDEXER_PROXY_ID );
		set_response( resp );
		proxies[ SCAN_PROXY_ID ] = -1;
	  }
	  bd->scans &= ~(1<<chno);
	}
  }
  return 0;
}

static unsigned short stop_channel( idx64_bd *bd, unsigned short chno ) {
  chandef *ch;
  
  assert( bd != 0 && chno < MAX_IDXR_CHANS );
  ch = &bd->chans[ chno ];

  /* Issue the stop command to the hardware */
  sbwr( ch->base_addr + 2, 0 ); /* Drive out 0 */

  /* Cancel pending requests */
  while ( ch->first != 0 ) dequeue( ch );
  bd->request &= ~(1 << chno);
  scan_setup( bd, chno, 0 );

  /* Clear all the TM bits */
  tm_status_set( ch->tm_ptr,
	  ch->scan_bit | ch->on_bit | ch->supp_bit, 0 );
  return EOK;
}

/* cancel proxy requests from intserv, delete proxies */
static void shutdown_boards( void ) {
  int i, j;
  idx64_bd *bd;

  for ( i = 0; i < MAX_IDXRS; i++ ) {
	bd = boards[i];
	if ( bd != 0 ) {
	  /* Stop each channel */
	  for ( j = 0; j < MAX_IDXR_CHANS; j++ )
		stop_channel( bd, j );
	  IntSrv_Int_detach( idx_defs[i].cardID );
	  qnx_proxy_detach( bd->proxy );
	}
  }
}

/* execute_cmd() executes the next command on the specified 
   channel's command queue. Except for scans, the command is then 
   dequeued. If no further commands are queued, the corresponding
   requests bit in the board structure is cleared.

   Scan strategy: in addition to requests, keep scans word
   for each board. On interrupt, clear request, on proxy,
   if request bit is clear, issue the command...
*/
static void execute_cmd( idx64_bd *bd, int chno ) {
  chandef *ch;
  ixcmdl *im;
  step_t curpos;

  ch = &bd->chans[ chno ];
  while ( ch->first != 0 ) {
	if ( ch->first->c.dir_scan == IX64_PRESET_POS ) {
	  sbwr( ch->base_addr + 4, ch->first->c.steps );
	} else if ( ch->first->c.dir_scan == IX64_SET_SPEED ) {
	  sbwr( ch->base_addr + 6, ch->first->c.steps & 0xF00 );
	} else break;
	dequeue( ch );
  }
  im = ch->first;
  if ( im != 0 ) {
	/* Translate ONLINE, OFFLINE and ALTLINE commands to TO commands */
	if (im->c.dir_scan == IX64_ONLINE) {
	  im->c.dir_scan = IX64_TO;
	  im->c.steps = ch->online;
	  tm_status_set( ch->tm_ptr, ch->supp_bit | ch->on_bit,
												ch->on_bit );
	} else if (im->c.dir_scan == IX64_OFFLINE) {
	  im->c.dir_scan = IX64_TO;
	  im->c.steps = ch->online + ch->offline_delta;
	  tm_status_set( ch->tm_ptr, ch->supp_bit | ch->on_bit,
								 ch->supp_bit );
	} else if (im->c.dir_scan == IX64_ALTLINE) {
	  im->c.dir_scan = IX64_TO;
	  im->c.steps = ch->online + ch->altline_delta;
	  tm_status_set( ch->tm_ptr, ch->supp_bit | ch->on_bit,
								 ch->supp_bit | ch->on_bit );
	} else
	  tm_status_set( ch->tm_ptr, ch->supp_bit | ch->on_bit, 0 );
  
	/* Translate IX64_TO to IX64_IN or IX64_OUT */
	if (im->c.dir_scan & IX64_TO) {
	  curpos = sbw( ch->base_addr + 4 );
	  im->c.dir_scan &= ~IX64_DIR;
	  if (curpos < im->c.steps) {
		im->c.dir_scan |= IX64_OUT;
		im->c.steps -= curpos;
	  } else {
		im->c.dir_scan |= IX64_IN; /* nop! */
		im->c.steps = curpos - im->c.steps;
	  }
	}

	if ( im->c.dir_scan & IX64_SCAN ) {
	  bd->request &= ~(1 << chno);
	  scan_setup( bd, chno, 1 );
	} else {
	  unsigned short addr;
	  
	  if ( (im->c.dir_scan & IX64_DIR) == IX64_OUT )
		addr = ch->base_addr + 2;
	  else addr = ch->base_addr;
	  sbwr( addr, im->c.steps );
	  dequeue( ch );
	}
  }
  if ( ch->first == 0 )
	bd->request &= ~(1 << chno);
}

/* service_board() is called when a board signals an interrupt
   and when a command is enqueued. It will query the board's
   status */
static void service_board( int bdno ) {
  idx64_bd *bd;
  unsigned int chno, mask;

  if ( bdno >= MAX_IDXRS || boards[bdno] == 0 )
	nl_error( 4, "Invalid bdno in service_board" );
  bd = boards[ bdno ];
  mask = bd->request & ~sbb( idx_defs[ bdno ].card_base );
  nl_error( -3, "svcbd bdno %d request %02X mask %02X scans %02X",
		  bdno, bd->request, mask, bd->scans );
  /* Mask should now have a non-zero bit for each channel
     which is ready to be serviced. */
  for ( chno = 0; bd->request != 0 && chno < MAX_IDXR_CHANS; ) {
	mask = 1 << chno;
	if ( mask &
		  bd->request &
		  ~sbb( idx_defs[ bdno ].card_base ) ) {
	  if ( bd->scans & mask )
		bd->request &= ~mask;
	  else
		execute_cmd( bd, chno );
	} else chno++;
  }
}

static unsigned short queue_request( idx64_cmnd *cmd ) {
  ixcmdl *cmdl;
  unsigned int bdno, chno;
  idx64_bd *bd;
  chandef *ch;
  
  bdno = cmd->drive / MAX_IDXR_CHANS;
  chno = cmd->drive % MAX_IDXR_CHANS;
  if ( bdno >= MAX_IDXRS || boards[bdno] == 0 )
	nl_error( 4, "Invalid bdno in queue_request" );
  bd = boards[ bdno ];
  ch = &bd->chans[ chno ];

  cmdl = malloc( sizeof( ixcmdl ) );
  if ( cmdl == 0 ) {
	nl_error( 1, "Out of memory in queue_request!" );
	return ENOMEM;
  }
  cmdl->next = NULL;
  cmdl->c = *cmd;
  if ( ch->first == 0 ) ch->first = cmdl;
  else ch->last->next = cmdl;
  ch->last = cmdl;
  bd->request |= (1 << chno);
  service_board( bdno );
  return EOK;
}

/* service_scan() is called from scan_proxy() to service an 
   ongoing scan. It is only called if the scans bit is set and
   the request bit is clear. After each drive, service_scan()
   sets the request bit, and it is cleared by execute_cmd()
   when the drive-completed interrupt is received. As such,
   though we don't actually read the board status here, we
   can be confident that the drive is complete and it is safe
   to drive the next step or to execute another command if
   the scan is complete.
*/
static void service_scan( idx64_bd *bd, int chno ) {
  chandef *ch;
  ixcmdl *im;
  
  ch = &bd->chans[chno];
  im = ch->first;
  assert( im != 0 );
  assert( ch->scan_bit == 0 || ch->tm_ptr != 0 );
  nl_error( -3, "svcscn chno %d steps %5d dsteps %5d", chno,
     im->c.steps, im->c.dsteps );
  if ( im->c.steps != 0 && ch->scan_bit != 0 &&
		( (*ch->tm_ptr) & ch->scan_bit ) == 0 ) {
	tm_status_set( ch->tm_ptr, ch->scan_bit, ch->scan_bit );
  } else if ( im->c.steps != 0 ) {
	unsigned short addr;
	  
	/* do drive and set request flag */
	if ( im->c.dsteps == 0 ) im->c.dsteps = 1;
	if ( im->c.dsteps > im->c.steps ) im->c.dsteps = im->c.steps;
	im->c.steps -= im->c.dsteps;
	if ( (im->c.dir_scan & IX64_DIR) == IX64_OUT )
	  addr = ch->base_addr + 2;
	else addr = ch->base_addr;
	sbwr( addr, im->c.dsteps );
	bd->request |= ( 1 << chno );
  } else if ( im->c.dsteps != 0 && ch->scan_bit != 0 ) {
	tm_status_set( ch->tm_ptr, ch->scan_bit, 0 );
	im->c.dsteps = 0;
  } else {
	/* all done: clear scans, dequeue and service next command */
	scan_setup( bd, chno, 0 );
	bd->request |= (1 << chno);
	dequeue( ch );
	execute_cmd( bd, chno );
  }
}

/* scan_proxy() is called when the scan proxy is received */
static void scan_proxy( void ) {
  idx64_bd *bd;
  unsigned short svc;
  int bdno, chno;

  for ( bdno = 0; bdno < MAX_IDXRS; bdno++ ) {
	bd = boards[bdno];
	if ( bd != 0 && ( svc = ( bd->scans & ~bd->request ) ) != 0 ) {
	  nl_error( -3, "scan_proxy svc %02X", svc );
	  for ( chno = 0; chno < MAX_IDXR_CHANS; chno++ ) {
		if ( svc & ( 1 << chno ) ) {
		  service_scan( bd, chno );
		}
	  }
	}
  }
}

/* drive_command() is called when a drive command message is
   received. It returns the status value which is to be returned
   in the Reply
*/
static unsigned short drive_command( idx64_cmnd *cmd ) {
  unsigned int bdno, chno;
  idx64_bd *bd;
  chandef *ch;

  bdno = cmd->drive / MAX_IDXR_CHANS;
  chno = cmd->drive % MAX_IDXR_CHANS;
  if ( bdno >= MAX_IDXRS || boards[bdno] == 0 ) return ENXIO;
  bd = boards[ bdno ];
  ch = &bd->chans[ chno ];
  
  if ( cmd->dir_scan < IX64_STOP )
	return queue_request( cmd );
  else switch ( cmd->dir_scan ) {
	case IX64_ONLINE:
	case IX64_OFFLINE:
	case IX64_ALTLINE:
	case IX64_PRESET_POS:
	case IX64_SET_SPEED:
	  return queue_request( cmd );
	case IX64_STOP:
	  return stop_channel( bd, chno );
	case IX64_MOVE_ONLINE_OUT:
	  ch->online += ch->online_delta; return EOK;
	case IX64_MOVE_ONLINE_IN:
	  ch->online -= ch->online_delta; return EOK;
	case IX64_SET_ONLINE:
	  ch->online = cmd->steps; return EOK;
	case IX64_SET_ON_DELTA:
	  ch->online_delta = cmd->steps; return EOK;
	case IX64_SET_OFF_DELTA:
	  ch->offline_delta = cmd->steps; return EOK;
	case IX64_SET_ALT_DELTA:
	  ch->altline_delta = cmd->steps; return EOK;
	case IX64_QUIT: /* should have been handled in operate() */
	default:
	  return ENOSYS; /* Unknown command */
  }
}

/* This is the main operational loop */
static void operate( void ) {
  pid_t who;
  idx64_msg im;
  idx64_reply rep;
  int done = 0, i;

  while ( ! done ) {
	who = Receive(0, &im, sizeof(im));
	if ( who == -1 ) {
	  nl_error( 1, "Error receiving" );
	} else {
	  for ( i = 0; i < N_PROXIES; i++ )
		if ( who == proxies[i] ) break;
	  if ( i < N_PROXIES ) {
		switch ( i ) {
		  case CC_PROXY_ID:
			nl_error( 0, "Received quit proxy" );
			done = 1;
			break;
		  case SCAN_PROXY_ID:
			scan_proxy();
			break;
		  default:
			service_board( i - BD_0_PROXY );
			break;
		}
	  } else {
		if ( im.type != IDX64_MSG_TYPE ) {
		  rep.status = ENOSYS;
		  nl_error( 1, "Unknown message type: 0x%04X", im.type );
		} else {
		  if ( im.ix.dir_scan == IX64_QUIT ) {
			done = 1;
			nl_error( 0, "Received Quit Request" );
			rep.status = EOK;
		  } else
			rep.status = drive_command( &im.ix );
		}
		Reply( who, &rep, sizeof( rep ) );
	  }
	}
  }
}

int main( int argc, char **argv ) {
  int name_id, resp;

  oui_init_options( argc, argv );
  init_boards();
  if ( idx64_cfg_string != 0 )
	config_channels( idx64_cfg_string );
  resp = set_response( 1 );
  tm_data = Col_send_init( "Idx64", tm_ptrs, sizeof(tm_ptrs) );
  proxies[ CC_PROXY_ID ] = cc_quit_request( 0 );
  set_response( resp );

  /* register name */
  name_id =
	qnx_name_attach( 0, nl_make_name( IDX64_NAME, 0 ) );
  if ( name_id == -1 )
	nl_error( 3, "Unable to attach name" );
  nl_error( 0, "Installed" );

  operate();

  /* cleanup: */
  qnx_name_detach( 0, name_id );
  shutdown_boards();
  Col_send_reset( tm_data );
  nl_error( 0, "Terminated" );
  return 0;
}
