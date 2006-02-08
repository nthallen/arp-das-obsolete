#include "qcliclt.h"
#include "cltsrvr.h"
#include "nortlib.h"

static Server_Def qclisrvr = { QCLISRVR, 1, 0, 2, 0, 0, 0, 0 };

int qcliclt_init( void ) {
  return CltInit(&qclisrvr);
}

int qcliclt_send( unsigned short cmd, unsigned short val,
			  unsigned short addr ) {
  qcliclt_msg msg;
  qcliclt_reply rep;
  unsigned short srv;
  msg.hdr = QCLISRVR_HDR;
  msg.cmd = cmd;
  msg.value = val;
  msg.addr = addr;
  srv = CltSend(&qclisrvr, &msg, &rep, sizeof(msg), sizeof(rep));
  if ( cmd == QCLISRVR_STATUS ) {
	return rep.cmdstatus != 0 ? 0xFFFF : rep.value;
  }
  return srv == 0 ? rep.cmdstatus : srv;
}

int qcli_set_dac( int n, unsigned short v) {
  if ( n < 0 || n >= QCLISRVR_N_DACS ) {
	nl_error( qclisrvr.response,
	  "DAC index out of range in qcli_set_dac()" );
	return(-1);
  } else {
	return qcliclt_send( QCLISRVR_SET_DAC, v, n );
  }
}
