#ifndef _DC_H_INCLUDED
#define _DC_H_INCLUDED

#include "dfs.h"
#include "reply.h"

#define OPT_DC_INIT "b:i:u"

#define DC_IS_RING (dc_topology == RING)
#define DC_IS_STAR (dc_topology == STAR)
#define DC_IS_BUS (dc_topology == BUS)

/* Function prototypes: */

/* Data Client library functions: */
extern int  DC_init(topology_type, long);
extern int  DC_init_options(int, char ** );
extern int  DC_operate(void);
extern reply_type DC_process_msg(void);
extern int  DC_bow_out(void);

/*  Data Client application functions: */
extern void DC_data(dbr_data_type *dr_data);
extern void DC_tstamp(tstamp_type *tstamp);
extern void DC_DASCmd(UBYTE1 type, UBYTE1 number);
extern reply_type DC_other(UBYTE1 *msg_ptr, pid_t sent_tid);

extern token_type DC_data_rows;
extern topology_type dc_topology;

#endif
