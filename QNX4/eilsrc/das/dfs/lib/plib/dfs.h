/* This include file consists of constants and structures that are shared
 * by all programs involved in DBR data information
 $Log$
 * Revision 2.1  1994/12/01  21:07:19  eil
 * dfs
 */
 
#ifndef DBR_H
#define DBR_H

#include <sys/types.h>
#include <limits.h>
#include "globmsg.h"
#include "port_types.h"
#include "qnx_ipc.h"

/* arbitrary size for data messages */
#define MAX_BUF_SIZE (PIPE_BUF>1000 ? 1000 : PIPE_BUF)

#define DG_NAME "dg"
#define DB_NAME "db"

/* token info; data request messages */
typedef UBYTE1 token_type;
#define TOKEN_SZ sizeof(token_type)

typedef struct {
  msg_hdr_type msg_hdr;
  token_type n_rows;
} msg_token_type;
#define MSG_TOKEN_SZ sizeof(msg_token_type)

/* Structure used for data */
typedef struct {
  token_type n_rows;
  UBYTE1 data[1];
} dbr_data_type;

/* Time stamp information */
typedef struct {
  UBYTE2 mfc_num;
  BYTE4 secs;
} tstamp_type;
#define TSTAMP_SZ sizeof(tstamp_type)

/* TM identifier */
typedef struct {
  BYTE1 ident[21]; /* 12345678.123,v 12.12 */
} tmid_type;

/* TM info */
typedef struct {
  tmid_type    tmid;
  UBYTE2 nbminf;
  UBYTE2 nbrow;
  UBYTE2 nrowmajf;
  UBYTE2 nsecsper; /* nrowsper/nsecsper = rows/sec */
  UBYTE2 nrowsper;
  UBYTE2 mfc_lsb;
  UBYTE2 mfc_msb;
  UBYTE2 synch;
  UBYTE2 isflag;
} tm_info_type;

#define ISF_INVERTED 1

/* Client Info upon Initialisation */
typedef struct {
  tm_info_type tm;    /* data info */
  UBYTE2 nrowminf;    /* number rows per minor frame */
  UBYTE2 max_rows;    /* max rows allowed to be sent in a message */
  BYTE2  next_tid;    /* tid to send to */
  UBYTE2 tm_started;  /* flow flag */
  tstamp_type t_stmp; /* current time stamp */
} dbr_info_type;
#define DBR_INFO_SZ sizeof(dbr_info_type)

/* Common Structure for what is received via IPC */
typedef struct {
  msg_hdr_type msg_hdr;
  union {
    dbr_data_type drd;
    tstamp_type tst;
    dascmd_type dasc;
    token_type n_rows;
    BYTE2 info; /* proxy ids */
  } u;
} dfs_msg_type;
#define DFS_MSG_SZ sizeof(dfs_msg_type)

/* topology protocol types */
typedef unsigned char topology_type;
#define RING 0
#define STAR 1
#define BUS 2

#define IS_RING(top) (top==RING)
#define IS_STAR(top) (top==STAR)
#define IS_BUS(top) (top==BUS)

/* Global variables declared in dfs_info.c */
extern dbr_info_type dbr_info;
#define tmi(x) dbr_info.tm.x
extern dfs_msg_type *dfs_msg;
extern int dfs_msg_size;
extern msg_hdr_type DFS_default_hdr;
extern pid_t dfs_who;
extern int dc_tok;
extern int dg_tok;

/* Global defined in library */
extern int ipc;

/* Global defined in eillib */
extern int breaksignal;

/* Functions declared in dfs.c */
extern int DFS_rec(topology_type top);
extern int DFS_init(topology_type top);

#endif
