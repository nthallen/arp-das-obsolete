/* snafu.c A channel type
   Each snafu channel is a separate open on the spreadsheet associated with
   a particular column > 0 (0 is always the X)
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "rtg.h"
#include "ssp.h"
#include "nortlib.h"

#define MIN_ALLOC 8
typedef struct {
  sps_ptr ssp;
  int column;
} chan_info;

static chan_info *chans = NULL;
static int n_chans = 0;

static void ssp_chan_delete(chandef *channel) {
  ss_close(chans[channel->channel_id].ssp);
  chans[channel->channel_id].ssp = -1;
}

static int ssp_pos_create(chandef *channel) {
  return ss_dup(chans[channel->channel_id].ssp);
}

static int ssp_pos_duplicate(chanpos *position) {
  return ss_dup(position->position_id);
}

static void ssp_pos_delete(chanpos *position) {
  ss_close(position->position_id);
}

static int ssp_pos_rewind(chanpos *position) {
  ss_jump_row(position->position_id, 0L);
  position->at_eof = 0;
  return 1;
}

static int ssp_pos_data(chanpos *position, double *X, double *Y) {
  sps_ptr ssp;
  int column;

  ssp = position->position_id;
  column = chans[position->channel->channel_id].column;
  if (ss_get(ssp, 0, X) < 0 || ss_get(ssp, column, Y) < 0) {
	position->at_eof = 1;
	return 0;
  }
  ss_next_row(ssp);
  return 1;
}

static int ssp_pos_move(chanpos *pos, long int index) {
  long int row;

  row = ss_row(pos->position_id);
  row += index;
  if (row < 0) {
	if (index > 0) row = -1;
	else row = 0;
  }
  if (ss_jump_row(pos->position_id, row) < 0)
	pos->at_eof = 1;
  else pos->at_eof = 0;
  return 0;
}

static chantype ss_chan_type = {
  ssp_chan_delete,
  ssp_pos_create,
  ssp_pos_duplicate,
  ssp_pos_delete,
  ssp_pos_rewind,
  ssp_pos_data,
  ssp_pos_move,
  { { 0, 0, -1, 0, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    /* X Reset Opts */
    { 0, 0, -1, 0, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },  /* Y Reset Opts */
  { { 0, 0, -1, 0, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },    /* X Default Opts */
    { 0, 0, -1, 0, -1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }   /* Y Default Opts */
};

static int allocate_channel_id(void) {
  int channel_id, i;

  for (channel_id = 0; channel_id < n_chans; channel_id++)
	if (chans[channel_id].ssp < 0) break;
  if (channel_id == n_chans) {
	n_chans = (n_chans > 0) ? n_chans * 2 : MIN_ALLOC;
	chans = realloc(chans, sizeof(chan_info)*n_chans);
	if (chans == 0)
	  nl_error(3, "Memory allocation failure in snafu:allocate_channel_id");
	for (i = channel_id; i < n_chans; i++) chans[i].ssp = -1;
  }
  return channel_id;
}

void ss_channels(char *name) {
  sps_ptr ssp;
  int n_cols, i, channel_id;
  chandef *channel;
  static int n_sps_s = 0, dflts_reset = 0;
  char ssname[_MAX_PATH];
  
  if (!dflts_reset) {
	ss_chan_type.DfltOpts = ss_chan_type.ResetOpts;
	dflts_reset = 1;
  }

  ssp = ss_open(name);
  if (ss_error(ssp)) return;
  n_sps_s++;

  /* get title of each column and create a channel */
  if (ss_chan_type.DfltOpts.X.units == 0) {
	char xtitle[40];
	
	if (ss_get_column_title(ssp, 0, xtitle) < 0 || xtitle[0] == '\0')
	  strcpy(xtitle, "X");
	ss_chan_type.DfltOpts.X.units = nl_strdup(xtitle);
	/* perhaps set a flag to remove this from Dflts? */
  }
  n_cols = ss_width(ssp);
  ss_name(ssp, ssname);
  for (i = 1; i < n_cols; i++) {
	char title[40];

	if (ss_get_column_title(ssp, i, title) >= 0) {
	  char name[_MAX_PATH+30];
	  
	  channel_id = allocate_channel_id();
	  chans[channel_id].column = i;
	  if (title[0] == '\0') sprintf(title, "[%d]", i);
	  sprintf(name, "%s/sps%s/[%d]", title, ssname, i);
	  channel = channel_create(name, &ss_chan_type, channel_id);
	  if (channel != 0) {
		chans[channel_id].ssp = ss_dup(ssp);
		dastring_update(&channel->opts.Y.units, title);
		if (channel->opts.X.limits.min > channel->opts.X.limits.max) {
		  ss_get_column_lower(ssp, 0, &channel->opts.X.limits.min);
		  ss_get_column_upper(ssp, 0, &channel->opts.X.limits.max);
		}
		if (channel->opts.Y.limits.min > channel->opts.Y.limits.max) {
		  ss_get_column_lower(ssp, i, &channel->opts.Y.limits.min);
		  ss_get_column_upper(ssp, i, &channel->opts.Y.limits.max);
		}
	  }
	}
  }
  ss_close(ssp);
}
