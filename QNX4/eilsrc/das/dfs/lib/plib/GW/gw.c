/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include "globmsg.h"
#include "eillib.h"
#include "gw.h"

reply_type DC_other(UBYTE1 *msg, pid_t who) {
  reply_type r=DAS_NO_REPLY;
  if (who) r=GW_dc_other(msg,who);
  if (r==DAS_NO_REPLY) {
    DG_process_msg();
  }
  return(r);
}

void DC_data(dbr_data_type *dr_data) {
  GW_dc_data(dr_data);
}

void DC_tstamp(tstamp_type *tstamp) {
  GW_dc_tstamp(tstamp);
}

void DC_DASCmd(UBYTE1 type, UBYTE1 number) {
  msg_dascmd_type d;
  GW_dc_DASCmd(type,number);
  d.msg_hdr=DASCMD;
  d.dasc.type=type;
  d.dasc.val=number;
  DC_other((char *)&d, dfs_who);
}

reply_type DG_other(UBYTE1 *msg, pid_t who) {
  return(GW_dg_other(msg,who));
}

void DG_DASCmd(UBYTE1 type, UBYTE1 val) {
  GW_dg_DASCmd(type,val);
}

int DG_get_data(token_type n_rows) {
  return(GW_dg_get_data(n_rows));
}

int GW_init_options(int argc, char **argv) {
  int r;
  if ((r=DC_init_options(argc,argv)) != 0) 
    msg(MSG_EXIT_ABNORM,"Can't initialise as a Data Client");
  if ((r=DG_init_options(argc,argv)) != 0)
    msg(MSG_EXIT_ABNORM,"Can't initialise as a Data Generator");
  return(r);
}

int GW_operate(void) {
  int i;
  i=DC_operate();
  return(i);
}

void GW_dg_s_data(token_type n_rows, unsigned char *data,\
               token_type n_rows1, unsigned char *data1) {
  DG_s_data(n_rows,data,n_rows1,data1);
}

void GW_dg_s_tstamp(tstamp_type *tstamp) {
  DG_s_tstamp(tstamp);
}

void GW_dg_s_dascmd(unsigned char type, unsigned char val) {
  DG_s_dascmd(type,val);
}

int  GW_dc_bow_out(void) {
  return(DC_bow_out());
}

