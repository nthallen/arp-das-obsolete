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
 Revision 1.2  2000/01/23 14:58:19  nort
 32-bit changes

 Revision 1.1  2000/01/23 14:56:55  nort
 Initial revision

 * Revision 1.6  1992/06/09  20:40:12  eil
 * dbr_info.seq_num
 *
 * Revision 1.4  1992/05/22  16:21:58  eil
 * merge eil and nort
 *
 * Revision 1.3  1992/05/21  18:52:56  nort
 * Split tm_info nrowsec into nrowsper and nsecsper.
 * Extended size of ident to 21 (More structure may be useful later).
 *
 * Revision 1.2  1992/05/20  17:37:58  nort
 * Changed prototype for DG_init().
 * Added prototype for DG_dac_in().
 * Added OPT_DG_* definitions.
 *
 */
 
#ifndef DBR_H
#define DBR_H

#include <sys/types.h> /* this is for pid_t and nid_t and time_t */

#define MAX_BUF_SIZE 1000 /* arbitrary size for DBRING message buffers with data */

/*
 * Message structures 
 */

/* token info */
typedef unsigned char token_type;

/* Structure used to transmit data around the dbr */
typedef struct {
  token_type n_rows;
  unsigned char data[1];
} __attribute__((packed)) dbr_data_type;

/* Time stamp information */
typedef struct {
  unsigned short mfc_num;
  time_t secs;
} __attribute__((packed)) tstamp_type;

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
  unsigned short nbminf;
  unsigned short nbrow;
  unsigned short nrowmajf;
  unsigned short nsecsper;
  unsigned short nrowsper;
  unsigned short mfc_lsb;
  unsigned short mfc_msb;
  unsigned short synch;
  unsigned short isflag;
} __attribute__((packed)) tm_info_type;
#define ISF_INVERTED 1

/* Reply to drinit */
typedef struct {
  tm_info_type tm;	    /* data info */
  unsigned short nrowminf;    /* number rows per minor frame */
  unsigned short max_rows;    /* maximum number of rows allowed to be sent in a message */
  unsigned short unused;	    /* tid to send to */
  unsigned short tm_started;  /* flow flag */
  tstamp_type  t_stmp;	    /* current time stamp */
  unsigned char mod_type;   /* module type */
} __attribute__((packed)) dbr_info_type;

/* Function prototypes: */

/* Data Client library functions: */
int  DC_init_options(int, char ** );
int  DC_operate(void);
int  DC_bow_out(void);

/*  Data Client application functions: */
void DC_data(dbr_data_type *dr_data);
void DC_tstamp(tstamp_type *tstamp);
void DC_DASCmd(unsigned char type, unsigned char number);
void DC_other(unsigned char *msg_ptr, pid_t sent_tid);

/* Global variables declared in dbr_info.c */
 extern dbr_info_type dbr_info;
 #define tmi(x) dbr_info.tm.x

#endif
