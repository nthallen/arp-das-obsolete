/* This include file consists of constants and structures that are shared
 * by all programs involved in DBR data information
 * Written by David Stahl
 * Modified May 21, 1991 by NTA
 * Modified May 22, 1991 by Eil
 * Modified May 23, 1991 by NTA extensively.
 * Modified June 20, 1991, took out R_OK
 * Modified Sep 26, 1991, by Eil, changed from ring to buffered ring. (dbr).
 * Modified by Eil 4/22/92, port to QNX 4.
 $Log$
 */
 
#ifndef DBR_H
#define DBR_H

#include <signal.h>
#include <globmsg.h>

#define MAX_BUF_SIZE 1000 /* arbitrary size for data messages */

/*
 * Message structures 
 */

#define DG_NAME "dg"
#define DB_NAME "db"

/* token info */
typedef unsigned char token_type;

/* mod types */
typedef unsigned char module_type;

#define IPC_ONLY(X) ((X) & 0x0F)

/* Structure used to transmit data around the dbr */
typedef struct {
  token_type n_rows;
  unsigned char data[1];
} dbr_data_type;

/* Time stamp information */
typedef struct {
  unsigned short mfc_num;
  time_t secs;
} tstamp_type;

/* dbr client initialization */
/* This will need some tweaking as we learn
   what RCS can and can't do for us
*/
typedef struct {
  char ident[21]; /* 12345678.123,v 12.12 */
} tmid_type;

/* nrowsper/nsecsper = rows/sec */
typedef struct {
  tmid_type    tmid;
  unsigned int nbminf;
  unsigned int nbrow;
  unsigned int nrowmajf;
  unsigned int nsecsper;
  unsigned int nrowsper;
  unsigned int mfc_lsb;
  unsigned int mfc_msb;
  unsigned int synch;
  unsigned int isflag;
} tm_info_type;
#define ISF_INVERTED 1

/* Reply to drinit */
typedef struct {
  tm_info_type tm;		    /* data info */
  unsigned int nrowminf;    /* number rows per minor frame */
  unsigned int max_rows;    /* maximum number of rows allowed to be sent in a message */
  int next_tid;	    		/* tid to send to */
  unsigned int tm_started;  /* flow flag */
  tstamp_type  t_stmp;	    /* current time stamp */
  module_type mod_type;		/* module type */
} dbr_info_type;

typedef struct {
	msg_hdr_type msg_type;
	int fromtid;
} hdr_type;

/* Common Structure for dg's and dc's */
typedef struct {
  hdr_type hdr;
  union {
    dbr_data_type drd;
    tstamp_type tst;
    dascmd_type dasc;
    token_type n_rows;
  } u;
} dfs_msg_type;

/* Global variables declared in dbr_info.c */
 extern dbr_info_type dbr_info;
 extern dfs_msg_type *dfs_msg;
 extern int dfs_msg_size;
 extern int msg_size;
 extern int my_ipc;
 extern sigset_t sigs;
 #define tmi(x) dbr_info.tm.x
 extern int dbr_breaksignal;

#ifndef __QNX__
#define _PPF_PRIORITY_REC
#define _PPF_SIGCATCH
#endif

#endif
