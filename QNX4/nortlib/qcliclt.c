#include "qcliclt.h"
#include "cltsrvr.h"
#include "nortlib.h"

static Server_Def qclisrvr = { QCLISRVR, 1, 0, 2, 0, 0, 0, 0 };

int qcliclt_init_v( Server_Def *qcliv ) {
  return CltInit( qcliv );
}

int qcliclt_init( void ) {
  return CltInit(&qclisrvr);
}

int qcliclt_send_v( Server_Def *qcliv, unsigned short cmd,
      unsigned short val, unsigned short addr ) {
  qcliclt_msg msg;
  qcliclt_reply rep;
  unsigned short srv;
  msg.hdr = QCLISRVR_HDR;
  msg.cmd = cmd;
  msg.value = val;
  msg.addr = addr;
  srv = CltSend(qcliv, &msg, &rep, sizeof(msg), sizeof(rep));
  if ( cmd == QCLISRVR_STATUS ) {
    return rep.cmdstatus != 0 ? 0xFFFF : rep.value;
  }
  return srv == 0 ? rep.cmdstatus : srv;
}

int qcliclt_send( unsigned short cmd, unsigned short val,
                          unsigned short addr ) {
  return qcliclt_send_v( &qclisrvr, cmd, val, addr );
}

int qcli_set_dac_v( Server_Def *qcliv, int n, unsigned short v) {
  if ( n < 0 || n >= QCLISRVR_N_DACS ) {
    nl_error( qcliv->response,
      "DAC index out of range in qcli_set_dac()" );
    return(-1);
  } else {
    return qcliclt_send_v( qcliv, QCLISRVR_SET_DAC, v, n );
  }
}

int qcli_set_dac( int n, unsigned short v) {
  return qcli_set_dac_v( &qclisrvr, n, v );
}

Server_Def *qcli_def( char *name ) {
  Server_Def *qcliv = new_memory(sizeof(Server_Def));
  qcliv->name = name;
  qcliv->expand = 1;
  qcliv->global = 0;
  qcliv->response = 2;
  qcliv->node = 0;
  qcliv->pid = 0;
  qcliv->connected = 0;
  qcliv->disconnected = 0;
  return qcliv;
}
