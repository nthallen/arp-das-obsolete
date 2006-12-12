#ifndef _GW_H_INCLUDED
#define _GW_H_INCLUDED

#include "dg.h"
#include "dc.h"

#define OPT_GW_INIT OPT_DC_INIT OPT_DG_INIT

/* Gateway Library functions */
/*extern int GW_init(void);*/
extern int GW_init_options(int, char **);
extern int GW_operate(void);
extern void GW_dg_s_data(token_type n_rows, unsigned char *data,
		      token_type n_rows1, unsigned char *data1);
extern void GW_dg_s_tstamp(tstamp_type *tstamp);
extern void GW_dg_s_dascmd(unsigned char type, unsigned char val);
extern int  GW_dc_bow_out(void);

/*  Gateway application functions: */
extern void GW_dc_data(dbr_data_type *dr_data);
extern void GW_dc_tstamp(tstamp_type *tstamp);
extern void GW_dc_DASCmd(UBYTE1 type, UBYTE1 number);
extern void GW_dg_DASCmd(UBYTE1 type, UBYTE1 number);
extern int  GW_dg_get_data(token_type n_rows);
extern reply_type GW_dc_other(UBYTE1 *msg_ptr, int who);
extern reply_type GW_dg_other(UBYTE1 *msg_ptr, int who);

#endif








