#ifndef DBRMIN_H
#define DBRMIN_H

/* token info */
typedef unsigned char token_type;

/* Structure used to transmit data around the dbr */
typedef struct {
  token_type n_rows;
  unsigned char data[1];
} __attribute__((packed)) dbr_data_type;

/* dbr client initialization */
/* This will need some tweaking as we learn
   what RCS can and can't do for us
*/
typedef struct {
  char ident[21]; /* 12345678.123,v 12.12 */
} dbr_tmid_type;

/* nrowsper/nsecsper = rows/sec */
typedef struct {
  dbr_tmid_type  tmid;
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
  tstamp_t       t_stmp;	    /* current time stamp */
  unsigned char mod_type;   /* module type */
} __attribute__((packed)) dbr_info_type;

/* Global variables declared in dbr_info.c */
extern dbr_info_type dbr_info;

#endif
