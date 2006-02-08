#ifndef QCLICLT_H_INCLUDED
#define QCLICLT_H_INCLUDED
#include "cltsrvr.h"

/* The QCLISRVR will Receive commands in a qcliclt_msg structure.
   It will reply with an unsigned short return value, provided
   that is enough information...
*/

#define QCLISRVR "QCLI"
#define QCLISRVR_HDR 'QC'
#define QCLISRVR_SELECT 0
#define QCLISRVR_RUN 1
#define QCLISRVR_STOP 2
#define QCLISRVR_CLEAR 3
#define QCLISRVR_SET_PARAM 4
#define QCLISRVR_STATUS 5
#define QCLISRVR_WSUB 6
#define QCLISRVR_SET_DAC 7
#define QCLISRVR_QUIT 8

#define QCLISRVR_N_DACS 4
#define QCLISRVR_N_PARAMS 3
#define QCLISRVR_P_TON 0
#define QCLISRVR_P_TOFF 1
#define QCLISRVR_P_TPRE 2

typedef struct {
  unsigned short hdr;
  unsigned short cmd;
  unsigned short value;
  unsigned short addr;
} qcliclt_msg;

typedef struct {
  unsigned short cmdstatus;
  unsigned short value;
} qcliclt_reply;

extern int qcliclt_init(void);
extern int qcliclt_send( unsigned short cmd, unsigned short val,
						unsigned short addr );
extern int qcli_set_dac( int n, unsigned short v );
#define qcli_select(n) qcliclt_send(QCLISRVR_SELECT, n, 0)
#define qcli_run() qcliclt_send(QCLISRVR_RUN, 0, 0)
#define qcli_stop() qcliclt_send(QCLISRVR_STOP, 0, 0)
#define qcli_clear() qcliclt_send(QCLISRVR_CLEAR, 0, 0)
#define qcli_set_ton(v) qcliclt_send(QCLISRVR_SET_PARAM, v, QCLISRVR_P_TON)
#define qcli_set_toff(v) qcliclt_send(QCLISRVR_SET_PARAM, v, QCLISRVR_P_TOFF)
#define qcli_set_tpre(v) qcliclt_send(QCLISRVR_SET_PARAM, v, QCLISRVR_P_TPRE)
#define qcli_read_status() qcliclt_send(QCLISRVR_STATUS, 0, 0)
#define qcli_write_subbus(a,v) qcliclt_send(QCLISRVR_WSUB, v, a)

#endif
