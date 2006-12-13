#include <sys/types.h>

/* defines */
/* rollover concerned comparisons: not needed yet */
#define LT(A,B,REF) ( (((B)>=(REF)) ? (A)<(B) : (A)>(REF) || (A)<(B) ) )
#define EQ(A,B) ( (A)==(B) )
#define GTE(A,B,REF) ( !(LT(A,B,REF)) )
#define LTE(A,B,REF) ( LT(A,B,REF) || EQ(A,B) )

typedef struct {
    pid_t who;
    token_type n_rows;
    unsigned char rflag;
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

typedef struct db_msg_type {
    msg_hdr_type hdr;
    token_type n_rows;
} db_msg_type;

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
extern int putrow;		    /* where to put next row of incoming data */
extern int requests;		    /* number of outstanding requests */
extern int stamps_and_cmds;	    /* number of stamps and cmds outstanding */
extern int got_quit;		    /* whether received DASCmd QUIT */
extern unsigned int n_clients;	    /* number of clients */

extern char *bfr;		    /* THE buffer */

/* sequence numbers: data rows and stamps/cmds: assume rollover wont occur */
extern unsigned long data_seq;	    /* the starting data sequence number */
extern unsigned long t_data_seq;    /* the tail data sequence number */
extern unsigned long list_seq;	    /* the starting guarenteed sequence number */
extern unsigned long t_list_seq;    /* the tail guarenteed sequence number */
extern unsigned long list_seen;	    /* minimum number of stamps seen */

/* vars needed for when bfr is a buffered client itself */
extern pid_t rq_who;		    /* requester */
extern db_msg_type rq_buf;	    /* requesters request message */

