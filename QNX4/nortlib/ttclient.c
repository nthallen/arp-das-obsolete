/* ttclient.c library interface for ttdriver
  This file could be modified to use cltsrvr.c functionality.
  That would require the CltSendmx() routine be written, which
  shouldn't be a big deal...
*/
#include <string.h>
#include <errno.h>
#include <sys/name.h>
#include <sys/kernel.h>
#include "nortlib.h"
#include "ttdriver.h"
char rcsid_ttclient_c[] = 
  "$Header$";

static pid_t ttdrv_pid = -1;

/* If the initialization fails, tt_init() invokes
   nl_error( nl_response, ... ) and returns non-zero.
   Hence, if nl_response >= 3 (default) tt_init() will
   not return on failure and can be assumed to have succeeded.
*/
unsigned short tt_init( void ) {
  if ( ttdrv_pid == -1 ) {
	char *name;
	
	name = nl_make_name( TT_NAME, 1 );
	ttdrv_pid = qnx_name_locate( 0, name, 0, 0 );
	if ( ttdrv_pid == -1 ) {
	  if ( nl_response )
		nl_error( nl_response, "Unable to locate ttdriver" );
	  return ~0;
	}
  }
  return 0;
}

/* returns non-zero if there was an error contacting ttdriver */
static int ttc_command(unsigned short function,
                       unsigned short address,
					   unsigned short data,
					   Reply_from_tt *rtt ) {
  Send_to_tt stt;

  if ( ttdrv_pid == -1 && tt_init() != 0 )
	return -1;
  stt.signature = 'tt';
  stt.function = function;
  stt.address = address;
  stt.data = data;
  if ( Send( ttdrv_pid, &stt, rtt, sizeof(stt), sizeof(*rtt) ) != 0 ) {
	if ( nl_response ) {
	  nl_error( nl_response, "Send to ttdriver failed: %s",
		  strerror( errno ) );
	}
	return -1;
  }
  if ( rtt->signature != 'tt' )
	nl_error( 4, "Garbled response from ttdriver" );
  return 0;
}

unsigned short tt_command( unsigned short function,
                           unsigned short address,
						   unsigned short data ) {
  Reply_from_tt rtt;
  
  if ( ttc_command( function, address, data, &rtt ) != 0 )
	return ~0;
  return rtt.value.shrt;
}

unsigned short tt_gc_read( gc_data_buf *gcbuf ) {
  struct _mxfer_entry xs, xr[2];
  Send_to_tt s_cmd;
  unsigned short rsig;

  if ( ttdrv_pid == -1 && tt_init() != 0 )
	return 0xFFFF;
  s_cmd.signature = 'tt';
  s_cmd.function = TTMSG_GC_READ;
  _setmx( &xs, &s_cmd, sizeof(s_cmd) );
  _setmx( &xr[0], &rsig, sizeof(rsig) );
  _setmx( &xr[1], gcbuf, sizeof( gc_data_buf ) );
  if ( Sendmx( ttdrv_pid, 1, 2, &xs, xr ) != 0 ) {
	if ( nl_response ) {
	  nl_error( nl_response, "Sendmx to ttdriver failed: %s",
		  strerror( errno ) );
	}
	return ~0;
  }
  if ( rsig != 'tt' )
	nl_error( 4, "Garbled response from ttdriver" );
  else if ( gcbuf->end_offset[0] == 0xFFFF )
	return ~0;
  return 0;
}

/* Returns ~0L on errors. */
unsigned long tt_gc_chan( unsigned short chan ) {
  Reply_from_tt rtt;
  
  if ( ttc_command( TTMSG_GC_CHAN, chan, 0, &rtt ) != 0 )
	return ~0L;
  return rtt.value.lng;
}
