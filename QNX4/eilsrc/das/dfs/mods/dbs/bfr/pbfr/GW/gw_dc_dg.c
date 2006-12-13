/* 
  Gateway DC and DG functions.
*/

/* includes */
#include <stdlib.h>
#include <string.h>
#include "dc.h"
#include "dg.h"
#include "eillib.h"

extern char *buf;
extern int bufrows;
static int put;

/* called when data is received from the dfs */
void DC_data(dbr_data_type *dr_data) {
  int i;
  int nr;
  char *d;
  static int mfr;
  static int over_uneven;

  i=0;
  nr=dr_data->n_rows;
  d=dr_data->data;
  if (put==bufrows) goto ret;

  if (over_uneven && mfr) {
    /* find mf boundary in data, if one */
    if (nr <= dbr_info.nrowminf-mfr) goto ret;
    else {
      nr=nr-(dbr_info.nrowminf-mfr);
      d=d+(dbr_info.nrowminf-mfr);
      over_uneven=0;
    }
  }
  i = min(bufrows-put, nr);
  memcpy(buf+(put*tmi(nbrow)),d, i*tmi(nbrow));
 ret:
  mfr = (mfr+dr_data->n_rows) % dbr_info.nrowminf;
  if (nr>(bufrows-put)) over_uneven=1;
  put+=i;
}

/* called when a time stamp received from dfs */
void DC_tstamp(tstamp_type *tstamp) {
/* this is also stored in dbr_info.t_stmp in DC */
}

/* called when a DASCmd is received from dfs */
void DC_DASCmd(unsigned char type, unsigned char number) {
  /* store for DG use */
  DG_s_dascmd(type, number);
}

reply_type DC_other(unsigned char *msg_ptr, int who) {
  return(DG_process_msg());
}

void DG_DASCmd(unsigned char type, unsigned char number) {
}

/* Called when data buffer receives an order for data from its clients. */
int DG_get_data(token_type n_rows) {
  static tstamp_type T;
  int i;
  if (dbr_info.t_stmp.mfc_num != T.mfc_num || 
      dbr_info.t_stmp.secs != T.secs) {
    T=dbr_info.t_stmp;
    DG_s_tstamp(&T);
    return 1;
  }
  i = min(n_rows, put);
  DG_s_data(i,buf,0,0);
  if (i<put) {
    put-=i;
    memcpy(buf,buf+(i*tmi(nbrow)),put);
  } else put=0;
  return 1;
}

reply_type DG_other(unsigned char *msg_ptr, pid_t who) {
  return(DAS_UNKN);
}
