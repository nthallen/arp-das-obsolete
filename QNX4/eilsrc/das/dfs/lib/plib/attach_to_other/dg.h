#ifndef _DG_H_INCLUDED
#define _DG_H_INCLUDED

#include "dfs.h"
#include "reply.h"

#define OPT_DG_INIT "n:j:BU"
#define OPT_DG_DAC_IN "f:"

#define DG_IS_RING (dg_topology == RING)
#define DG_IS_STAR (dg_topology == STAR)
#define DG_IS_BUS (dg_topology == BUS)

/* Function prototypes: */

/* Data Generator library functions: */
extern int  DG_init(int start_after_inits_num, int delay_in_ms, \
		    topology_type mt);
extern int  DG_dac_in(int argc, char **argv);
extern int  DG_init_options(int argc, char **argv);
extern int  DG_operate(void);
extern reply_type DG_process_msg(void);
extern void DG_s_tstamp(tstamp_type *tstamp);
extern void DG_s_dascmd(UBYTE1 type, UBYTE1 val);
extern void DG_s_data(token_type n_rows,UBYTE1 *dta,token_type n_rows1, \
		      UBYTE1 *dta1);

/* Data Generator application functions: */
extern void       DG_DASCmd(UBYTE1 type, UBYTE1 val);
extern reply_type DG_other(UBYTE1 *msg_ptr, pid_t sent_tid);
extern int        DG_get_data(token_type n_rows);

extern topology_type dg_topology;

#define DASCQSIZE 5

#endif

