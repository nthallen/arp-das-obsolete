/* seq_ss.c
 * $Log$
 * Revision 1.1  1993/12/02  20:52:45  nort
 * Initial revision
 *
 */
#include "nortlib.h"
#include "ssp.h"
#include "seq_ss.h"
#pragma off (unreferenced)
  static char rcsid[] =
	"$Id$";
#pragma on (unreferenced)


static sps_ptr ssp = -1;
static int seq_no = 0;
static int n_sps_cols = 0;

static struct scanlims {
  double min, max;
} *limits;

static void reset_limits(int n_columns) {
  int i;

  if (limits != NULL && n_columns != n_sps_cols)
	free_memory(limits);
  n_sps_cols = n_columns;
  limits = new_memory(n_sps_cols * sizeof(struct scanlims));
  for (i = 0; i < n_sps_cols; i++) {
	limits[i].min = limits[i].max = non_number;
  }
}

static void check_limit(int col, double value) {
  if (is_number(limits[col].min)) {
	if (value < limits[col].min) limits[col].min = value;
	else if (value > limits[col].max) limits[col].max = value;
  } else limits[col].min = limits[col].max = value;
}

static void store_limits(void) {
  int i;
  
  for (i = 0; i < n_sps_cols; i++) {
	ss_set_column_upper(ssp, i, limits[i].max);
	ss_set_column_lower(ssp, i, limits[i].min);
  }
}

void store_seq_ss(double v0, int col, double val) {
  if (ssp >= 0) {
	ss_insert_value(ssp, v0, 0.);
	check_limit(0, v0);
	ss_set(ssp, col, val);
	check_limit(col, val);
  }
}

sps_ptr open_seq_ss(char *prefix, int n_columns) {
  char name[20];
  
  sprintf(name, "%s%04d", prefix, seq_no);
  ssp = ss_create(name, 1, n_columns, 0);
  if (ssp < 0) nl_error(1, "Unable to open spreadsheet %s", name);
  else reset_limits(n_columns);
  return(ssp);
}

/* Returns -1 if the spreadsheet is to be discarded, else returns
   the sequence number
 */
int close_seq_ss(int save_it) {
  if (ssp >= 0) {
	if (save_it) store_limits();
	ss_close(ssp);
	ssp = -1;
	if (save_it) return(seq_no++);
  }
  return(-1);
}
