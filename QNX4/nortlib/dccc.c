/* Definitions for sending to DCCC */
#include "cltsrvr.h"
#include "globmsg.h"
#include "dccc.h"
#include "nortlib.h"
char rcsid_dccc_c[] =
  "$Header$";

static Server_Def DCdef = { DCCC, 1, 0, 1, 0, 0, 0, 0 };

unsigned short DigSelect( unsigned short cmd, unsigned short val ) {
  struct {
	unsigned char type;
	unsigned char dc_type;
	unsigned char cmd;
	unsigned short value;
  } dcmd;
  unsigned char rv;
	  
  rcsid_dccc_c[0] = rcsid_dccc_c[0];
  dcmd.type = DASCMD;
  dcmd.dc_type = DCT_DCCC;
  dcmd.cmd = cmd;
  dcmd.value = val;
  if ( !CltSend( &DCdef, &dcmd, &rv, sizeof(dcmd), sizeof(rv) ) ) {
	if ( rv != DAS_OK )
		nl_error( 2, "DCCC returned %d\n", rv );
  }
  return val;
}

/*
=Name DigSelect(): Send SELECT command to DCCC
=Subject Client/Server
=Subject Data Collection

=Synopsis
#include "nortlib.h"

unsigned short DigSelect( unsigned short cmd, unsigned short val );

=Description

Sends the specified SELECT command to DCCC. This function should
work with SET commands as well.

=Returns

DigSelect() returns the value argument passed to it.

=SeeAlso

=send_dascmd=(), =Client/Server= functions.

=End
*/
