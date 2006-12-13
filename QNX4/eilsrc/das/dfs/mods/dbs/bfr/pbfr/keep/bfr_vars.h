#include <sys/types.h>
typedef struct {
  pid_t who;
  unsigned long seq;
  unsigned long list_seen;
  unsigned long data_lost;
} arr;

typedef struct ll {
  msg_hdr_type hdr;
  unsigned long seq;
  union {
    dascmd_type c;
    tstamp_type s;
  } u;
  struct ll *next;
} llist;

/* **************** */
/* global variables */
/* **************** */

/* command line option string */
extern char *opt_string;	    /* global options string */

/* data structures */
extern arr clients[];		    /* clients array */
extern llist *list;		    /* current commands and stamps */
extern llist *t_list;		    /* tail of list of commands stamps */

/* buffer status vars: all integers */
extern int bfr_sz_bytes;	    /* buffer size in bytes */
extern int bfr_sz_rows;		    /* buffer size in rows */
extern int startrow;		    /* row of oldest data */
extern int putrow;		    /* where put next row of incoming data */
extern int stamps_and_cmds;	    /* number of stamps and cmds outstanding */
extern int got_quit;		    /* whether received DASCmd QUIT */
extern unsigned int n_clients;	    /* number of clients */

extern char *bfr;		    /* THE buffer */

/* sequence numbers: data rows and stamps/cmds: assume rollover wont occur */
extern unsigned long data_seq;	    /* the starting data sequence number */
extern unsigned long t_data_seq;    /* the tail data sequence number */
extern unsigned long list_seq;	    /* starting guarenteed sequence number */
extern unsigned long t_list_seq;    /* the tail guarenteed sequence number */

/* functions */
extern int compare(pid_t *w, arr *a);
extern unsigned long send_cmd_or_stamp(arr *a,msg_hdr_type h);
extern int send_data(token_type nrows, arr *a);

extern llist *dc_cmd;

#define FIND_CLIENT(X) \
  (arr *)bsearch(&(X), clients, n_clients, sizeof(arr), compare)
