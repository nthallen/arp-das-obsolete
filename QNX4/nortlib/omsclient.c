/* omsclient.c interfaces to omsdrv which interfaces to
   Oregon Micro Systems PC68 Motor Controller Board
*/
#include <string.h>
#include <stdarg.h>
#include "nortlib.h"
#include "omsdrv.h"
#include "cltsrvr.h"
static char rcsid_oms_c[] =
  "$Header$";

static Server_Def OMSDef = { OMS_NAME, 1, 0, 2, 0, 0, 0, 0 };

int oms_init( void ) {
  rcsid_oms_c[0] = rcsid_oms_c[0];
  return CltInit( &OMSDef );
}

/* oms_command returns
   0 on success,
   1 on unrecognized command
   -1 if unable to reach server
   On a successful read, the requested data is written into
   the specified buffer, which must be at least OMS_CMD_MAX
   bytes long.
*/
int oms_command( USHRT function, char *cmd, USHRT n_req, char *buf ) {
  Send_hdr_to_oms S;
  Reply_hdr_from_oms R;
  struct _mxfer_entry rx[2], sx[2];
  unsigned rparts = 1;
  int len;
  
  if ( cmd == NULL ) len = 0;
  else len = strlen(cmd) + 1;
  if ( len > OMS_CMD_MAX )
	nl_error( 3, "OMS Command string too long or NULL" );
  S.signature = OMS_SIG;
  S.function = function;
  S.n_req = n_req;
  _setmx( &sx[0], &S, sizeof(S) );
  _setmx( &sx[1], cmd, len );
  
  _setmx( &rx[0], &R, sizeof(R) );
  if ( buf != NULL ) {
	_setmx( &rx[1], buf, OMS_CMD_MAX );
	rparts++;
  }
  
  if ( CltSendmx( &OMSDef, 2, rparts, sx, rx ) == 0 ) {
	if ( R.signature != OMS_SIG )
	  nl_error( 4, "Garbled response from omsdrv" );
	return R.status;
  } else return -1;
}

int oms_fprintf(char *fmt, ...) {
  va_list arg;
  char cmdbuf[OMS_CMD_MAX];

  va_start(arg, fmt);
  vsprintf(cmdbuf, fmt, arg);
  va_end(arg);
  return(oms_write(cmdbuf));
}

/* supports MR, MA, LP, VL */
int oms_fcommand( char axis, char *cmd, long distance, int go ) {
  return oms_fprintf( "A%c%s%ld;%s",
		  axis, cmd, distance, go ? "GO" : "" );
}

/*
=Name oms_command(): Send command to OMS Driver
=Subject Driver Interfaces
=Name oms_read(): Read from OMS Driver
=Subject OMS PC68 Motor Controller Driver
=Name oms_write(): Write to OMS Driver
=Subject OMS PC68 Motor Controller Driver
=Name oms_init(): Initiate communication with OMS Driver
=Subject OMS PC68 Motor Controller Driver
=Subject Startup
=Name oms_shutdown(): Ask resident OMS Driver to quit
=Subject OMS PC68 Motor Controller Driver
=Subject Shutdown

=Synopsis
#include "omsdrv.h"
int oms_init( void );
int oms_command( USHRT function, char *cmd, USHRT n_req, char *buf );
int oms_shutdown( void );
int oms_read(char *cmd, USHRT n_req, char *buffer );
int oms_write( char *cmd );

=Description

  <P>These functions provide basic client access to omsdrv, the
  driver for the Oregon Micro Systems PC68 Motor Controller
  Board.

  <P>oms_init() provides the standard client/server
  initialization. As usual, it is optional, but calling it at
  startup might flag an error condition sooner than if you wait
  until the first OMS command to find out.</P>
  
  <P>oms_command() is the main routine for client communication
  with the OMS driver, omsdrv. oms_shutdown(), oms_read() and
  oms_write() are all implemented via macros which invoke
  oms_command(). The function codes are defined in omsdrv.h.
  cmd is the exact command string that should be written out to
  the PC68 board. n_req specifies the number of atomic responses
  expected from the board, and is only applicable for read
  requests. (If you expect results you wish to ignore, we'll need
  to amend this rule.) buf is a buffer into which the result
  string is written, stripped of lf-cr prefixes and suffixes. It
  is the client's responsibility to make sense out of the string.
  </P>
  
  <P>oms_shutdown() attempts to locate a resident omsdrv and
  request that it shut down.</P>
  
  <P>oms_read() is used to issue a command to the PC68 when a
  reply is expected. It uses all the arguments of oms_command().
  </P>
  
  <P>oms_write() is used to issue commands to the PC68 which
  do not produce a reply.

=Returns

  All functions return 0 on success. oms_command() returns 1 if
  the function code is unrecognized by the server and -1 if the
  server cannot be located or the reply does not have the correct
  signature. The response level for this driver is 2, so most
  errors will produce an error complaint but won't be fatal,
  although oms_command() will abort no matter what if you pass it
  invalid arguments.
  
=SeeAlso

  =Driver Interfaces=, =Startup= functions.
  
=End
*/
