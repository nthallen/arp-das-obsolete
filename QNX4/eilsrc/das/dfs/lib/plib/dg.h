#ifndef _DG_H_INCLUDED
#define _DG_H_INCLUDED

#include <dfs.h>
#include <reply.h>

#define OPT_DG_INIT "n:j:z:x"
#define OPT_DG_DAC_IN "f:"

/* generator types */
#define DG 0x40
#define DRG 0x40
#define DSG 0xC0
#define IS_DG(X) ((X) & DG)
#define DG_ONLY(X) ((X) & 0xC0)

/* Function prototypes: */

/* Data Generator library functions: */
int  DG_init(int start_after_inits_num, int delay_in_ms, module_type mt);
int  DG_dac_in(int argc, char **argv);
int  DG_init_options(int argc, char **argv);
int  DG_operate(void);
int  DG_process_msg(int who);
void DG_s_data(token_type n_rows, unsigned char *data, token_type n_rows1, unsigned char *data1);
void DG_s_tstamp(tstamp_type *tstamp);
void DG_s_dascmd(unsigned char type, unsigned char val);

/* Data Generator application functions: */
void  		DG_DASCmd(unsigned char type, unsigned char val);
reply_type  DG_other(unsigned char *msg_ptr, int sent_tid, int *msg_size);
int  		DG_get_data(token_type n_rows);

extern token_type DG_rows_requested;

#endif

