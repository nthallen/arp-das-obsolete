/*
 * Message types
 * Written by NTA
 * Modified June 12, 1991 by Eil, changed return code names
 * Modified July 29, 1991 by Eil, to include dcccmult msg type
 * Modified Aug 7, 1991 by Eil, to change dcccmult to multcmd
 $Id$
 $Log$
 * Revision 1.13  1993/08/24  16:04:01  eil
 * added DATA and STAMP universal msg types
 *
 * Revision 1.12  1993/02/09  15:44:38  eil
 * generalised
 *
 * Revision 1.11  1992/09/23  14:35:01  eil
 * added the SOLDRV_PROXY 's
 *
 * Revision 1.10  1992/09/22  19:51:11  eil
 * added SOLDRV_E - SOLDRV_J
 *
 * Revision 1.9  1992/09/22  18:47:46  eil
 * changed name of SOL_PROXY to SOLDRV_PROXY to avoid clash with codes.h.
 *
 * Revision 1.8  1992/09/22  17:42:29  eil
 * a move around,  SOLDRV_?'s.
 *
 * Revision 1.7  1992/09/02  13:40:48  eil
 * added INDEXER_MSG, rid DG_regulate
 *
 * Revision 1.6  1992/08/21  19:57:55  eil
 * add MEMO msg
 *
 * Revision 1.5  1992/07/31  14:51:59  eil
 * added extra DCT_SOLDRV's and extra MULTCMD
 *
 * Revision 1.4  1992/07/16  14:25:35  eil
 * added msg_hdr_type, dascmd_type, MAX_MSG_SIZE etc
 *
 * Revision 1.3  1992/06/10  17:01:49  nort
 * Removed DGSignal: not global
 *
 * Revision 1.2  1992/06/10  16:44:04  nort
 * Changed DRTOKEN to DCTOKEN
 * Added DGRegulate and DGSignal
 *
 * Revision 1.1  1992/05/19  17:53:12  nort
 * Initial revision
 *
 */

#ifndef GLOBMSG_H
#define GLOBMSG_H

#include "reply.h"
#include "memo.h"

#define MAX_MSG_SIZE MEMO_MSG_MAX /* maximum msg size for DAS, excluding data msgs */

typedef unsigned char msg_hdr_type;
/* DAS command type */
typedef struct {
  unsigned char type;
  unsigned char val;
} __attribute__((packed)) dascmd_type;

/* Universal message headers
*/
#define DEATH	    0	/* for task administrators */ 
#define DASCMD      1   /* DAS command msg header */
#define DATA		25  /* for msgs carrying data outside ring */
#define STAMP		26	/* for msgs carrying time stamps outside ring */
#define INIT		27  /* for msgs carrying init info outside ring */

/* data buffered ring specific message types */
#define DCTOKEN     2   /* used by last client to pass to DG */
#define DCDATA      3   /* type used to transmit data around data buf ring */
#define DCDASCMD    4   /* DBRing version of DASCMD */
#define TSTAMP      5   /* data type used to indicate a time stamp */
#define DCINIT      6   /* data type used to initialize a task into dbr */

#define CCReg_MSG   2   /* Message is a Command Control Registration Request */
#define MEMO_MSG    MEMO_HDR   /* Message is a message */
#define SC_MULTCMD  11  /* dccc multiple set/strobe/step command */
#define INDEXER_MSG 12
#define TIME_SET    13  /* to set a time */
#define DC_MULTCMD  14
#define SOLDRV_PROXY_A   15	/* Solenoid proxy hook */
#define SOLDRV_PROXY_B   16
#define SOLDRV_PROXY_C   17
#define SOLDRV_PROXY_D   18
#define SOLDRV_PROXY_E   19
#define SOLDRV_PROXY_F   20
#define SOLDRV_PROXY_G   21
#define SOLDRV_PROXY_H   22
#define SOLDRV_PROXY_I   23
#define SOLDRV_PROXY_J   24

#define MAX_GLOBMSG 128


/* Universal message types */
#define TIME_START  0	/* after TIME_SET */
#define TIME_END    1	/* after TIME_SET */


/* Universal DASCmd Types:
   These are the command types which are experiment-independent.
   DCT stands for DasCmd Type. DCV stands for DasCmd Value and is
   used for (logically) discrete commands.
*/
#define DCT_QUIT 0
#define DCV_QUIT 0

#define DCT_TM   1
#define DCV_TM_START  0
#define DCV_TM_END    1
#define DCV_TM_CLEAR  2
#define DCV_TM_SUSLOG 3
#define DCV_TM_RESLOG 4

#define DCT_DCCC 2
#define DCT_SCDC 3
#define DCT_SOLDRV_A 4
#define DCT_SOLDRV_B 5
#define DCT_SOLDRV_C 6
#define DCT_SOLDRV_D 7
#define DCT_SOLDRV_E 8
#define DCT_SOLDRV_F 9
#define DCT_SOLDRV_G 10
#define DCT_SOLDRV_H 11
#define DCT_SOLDRV_I 12
#define DCT_SOLDRV_J 13

/* DAS reply codes = standard reply codes */
#define DAS_OK REP_OK
#define DAS_UNKN REP_UNKN
#define DAS_BUSY REP_BUSY

#endif
