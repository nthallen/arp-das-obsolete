#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/name.h>
#include <stddef.h>
#include "nortlib.h"
#include "oui.h"
#include "globmsg.h"
#include "ttdriver.h"
#include "ttintern.h"

static tt_done = 0;

static void gc_read( pid_t who ) {
  Reply_from_gc buf;
  unsigned int samples;
  
  buf.signature = 'tt';
  samples = tti_gc_read( &buf.gcbuf );
  samples *= sizeof(unsigned long);
  samples += offsetof( Reply_from_gc, gcbuf.samples[0] );
  /* ( N_GC_CHANNELS - 1 ) * sizeof( unsigned short ); */
  Reply( who, &buf, samples );
}

/* tt_command() processes all the board-specific tt_* commands.
   It is responsible for replying to all messages
*/
static void tti_command( pid_t who, Send_to_tt *cmd ) {
  Reply_from_tt rep;

  if ( cmd->signature != 'tt' ) reply_byte( who, REP_UNKN );
  else switch ( cmd->function ) {
	case TTMSG_READ_ATOD:
	  rep.value.shrt = tti_read_atod( cmd->address );
	  break;
	case TTMSG_WRITE_DIGITAL:
	  rep.value.shrt = tti_write_digital( cmd->address, cmd->data );
	  break;
	case TTMSG_READ_DIGITAL:
	  rep.value.shrt = tti_read_digital( cmd->address );
	  break;
	case TTMSG_GC_RESET:
	  rep.value.shrt = tti_gc_reset();
	  break;
	case TTMSG_GC_BYTE:
	  rep.value.shrt = tti_gc_byte();
	  break;
	case TTMSG_GC_READ:
	  gc_read( who );
	  return;
	case TTMSG_GC_CHAN:
	  rep.value.lng = tti_gc_chan( cmd->address );
	  break;
	case TTMSG_DAC_OUT:
	  rep.value.shrt = tti_dac_out( cmd->address, cmd->data );
	  break;
	case TTMSG_DAC_IN:
	  rep.value.shrt = tti_dac_in( cmd->address );
	  break;
	case TTMSG_DAC_ENBL:
	  rep.value.shrt = tti_dac_enbl( cmd->address );
	  break;
	case TTMSG_SCDC:
	  rep.value.shrt = tti_scdc_command( cmd->address );
	  break;
	case TTMSG_QUIT:
	  rep.value.shrt = 0;
	  tt_done = 1;
	  break;
	default:
	  rep.value.shrt = ~0;
	  break;
  }
  rep.signature = 'tt';
  Reply( who, &rep, sizeof( rep ) );
}

/* scdc_command() processes all the DASCMDs, servicing only
   DCT_SCDC and otherwise replying REP_UNKN. It will reply
   REP_UNKN also if the command value is out of range.
   It is responsible for replying to all messages.
*/
static void scdc_command( pid_t who, unsigned char *cmd ) {
  if ( cmd[1] == DCT_SCDC ) {
	if (tti_scdc_command( cmd[2] ) == 0 ) {
	  nl_error( -3, "SCDC command %u", cmd[2] );
	  reply_byte( who, REP_OK );
	  return;
	}
	nl_error( -2, "SCDC command out of range: %u", cmd[2] );
  } else nl_error( 1, "Unexpected DASCMD Type %u", cmd[1] );
  reply_byte( who, REP_UNKN );
}

/* scdc_multcmd() processes the SC_MULTCMDs. It will reply
   REP_UNKN if it encounters an unknown command code.
   It is responsible for replying to all messages.
*/
static void scdc_multcmd( pid_t who, unsigned char *cmd ) {
  int n = *++cmd;
  while ( n-- > 0 ) {
	if ( tti_scdc_command( *++cmd ) != 0 ) {
	  nl_error( -2, "MULTCMD command out of range: %d", *cmd );
	  reply_byte( who, REP_UNKN );
	  return;
	} else nl_error( -4, "MULTCMD %d", *cmd );
  }
  nl_error( -3, "MULTCMD received" );
  reply_byte( who, REP_OK );
  return;
}

/* operate() houses the main Receive Loop for ttdriver */
void operate( void ) {
  pid_t who;
  union {
	Send_to_tt tt;
	unsigned char scdc[ MAX_MSG_SIZE ];
  } cmd;
  
  while ( ! tt_done ) {
	who = Receive( 0, &cmd, sizeof( cmd ) );
	if ( who != -1 ) {
	  switch ( cmd.scdc[0] ) {
		case 't':        tti_command( who, &cmd.tt );    break;
		case DASCMD:     scdc_command( who, &cmd.scdc ); break;
		case SC_MULTCMD: scdc_multcmd( who, &cmd.scdc ); break;
		default:         reply_byte( who, REP_UNKN );    break;
	  }
	}
  }
}

int main( int argc, char **argv ) {
  int ttname_id, scname_id;
  char *name;
  
  oui_init_options( argc, argv );

  /* attach ttdriver name */
  name = nl_make_name( TT_NAME, 1 );
  ttname_id = qnx_name_attach( 0, name );
  if ( ttname_id == -1 )
	nl_error( 3, "Unable to attach name %s", name );

  /* attach scdc name */
  name = nl_make_name( "scdc", 0 );
  scname_id = qnx_name_attach( 0, name );
  if ( scname_id == -1 )
	nl_error( 1, "Unable to attach name %s", name );

  /* perform any hardware initializations */
  init_timer();

  nl_error( 0, "Initialized" );
  operate();
  nl_error( 0, "Shutting Down" );
  
  qnx_name_detach( 0, ttname_id );
  if ( scname_id != -1 )
	qnx_name_detach( 0, scname_id );

  return 0;
}
