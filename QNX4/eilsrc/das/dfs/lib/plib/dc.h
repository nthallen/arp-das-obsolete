#ifndef _DC_H_INCLUDED
#define _DC_H_INCLUDED

#include <dfs.h>
#include <reply.h>

#define OPT_DC_INIT "b:i:"

/* client */
#define DC 0x10
#define DRC 0x10
#define DSC 0x30
#define IS_DC(X) ((X) & DC)
#define DC_ONLY(X) ((X) & 0x30)

/* Function prototypes: */

/* Data Client library functions: */
int  DC_init(int, long);
int  DC_init_options(int, char ** );
int  DC_operate(void);
int  DC_process_msg(int who);
int  DC_bow_out(void);

/*  Data Client application functions: */
void DC_data(dbr_data_type *dr_data);
void DC_tstamp(tstamp_type *tstamp);
void DC_DASCmd(unsigned char type, unsigned char number);
reply_type DC_other(unsigned char *msg_ptr, pid_t sent_tid, int *msg_size);

extern token_type DC_data_rows;
#endif
