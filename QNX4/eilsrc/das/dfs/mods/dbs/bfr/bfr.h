#include <sys/types.h>

/* defines */
#define LT(A,B,REF) ( (((B)>=(REF)) ? (A)<(B) : (A)>(REF) || (A)<(B) ) )
#define EQ(A,B) ( (A)==(B) )
#define GTE(A,B,REF) ( !(LT(A,B,REF)) )
#define LTE(A,B,REF) ( LT(A,B,REF) || EQ(A,B) )

#define MAX_STAR_CLIENTS 10
#define DEF_BFR_SIZE 5120
#define MAX_BFR_SIZE 32000
#define CHECK_CLIENTS 50    /* check each registered client for existance
				when the total number of stamps and cmds
				outstanding, reaches this level.
			    */

typedef struct {
    pid_t who;
    unsigned long seq;
    token_type n_rows;
    unsigned char rflag;
    unsigned long list_seen;
    unsigned int data_lost;
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

/* global variables */
extern arr clients[];		    /* clients array */
extern llist *list;		    /* current commands and stamps */
extern llist *t_list;		    /* tail of list of commands stamps */
extern char *opt_string;	    /* global options string */
extern unsigned int bfr_sz_bytes;   /* buffer size in bytes */
extern int bfr_sz_rows;		    /* buffer size in rows */
extern int startrow;		    /* row of oldest data */
extern int putrow;		    /* where to put next row of incoming data */
extern unsigned int stamps_and_cmds;/* number of stamps and cmds outstanding */
extern char *bfr;		    /* THE buffer */
extern unsigned long data_seq;	    /* the starting data sequence number */
extern unsigned long t_data_seq;    /* the tail data sequence number */
extern unsigned long list_seq;	    /* the starting guarenteed sequence number */
extern unsigned long t_list_seq;    /* the tail guarenteed sequence number */
extern unsigned char requests;	    /* number of outstanding requests */
extern unsigned long list_seen;	    /* minimum number of stamps seen */
extern unsigned int n_clients;	    /* number of clients */
extern unsigned char got_quit;	    /* whether received DASCmd QUIT */
