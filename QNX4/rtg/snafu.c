/* snafu.c A channel type
   Each snafu channel is a separate open on the spreadsheet associated with
   a particular column > 0 (0 is always the X)
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
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

static int ss_dup_chk( int ssp ) {
  ssp = ss_dup( ssp );
  if (ss_error(ssp))
	nl_error( 4, "Snafu ssp error %d", ssp );
  return ssp;
}

static int ssp_pos_create(chandef *channel) {
  return ss_dup_chk(chans[channel->channel_id].ssp);
}

static int ssp_pos_duplicate(chanpos *position) {
  return ss_dup_chk(position->position_id);
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

/* This routine generates script entries to reopen all the currently
   open spreadsheets. At present, this will not necessarily duplicate
   the current configuration, since channels may have been deleted
   since the spreadsheet was opened and these channels will reappear
   when the spreadsheet is reopened, but that's life, no?
*/
typedef struct nm_list {
  char *name;
  struct nm_list *next;
} nmlist;
static void ss_report(void) {
  nmlist *nml = NULL, *nmp;
  int i;
  char *p;
  
  for (i = 0; i < n_chans; i++) {
	if (chans[i].ssp >= 0) {
	  char filename[PATH_MAX];
	  
	  /* get the name into a local buffer */
	  ss_name(chans[i].ssp, filename);
	  /* eliminate the ".sps" extension */
	  if (p = strrchr( filename, '.' ) )
		*p = '\0';
	  /* compare against all the existing names */
	  for (nmp = nml; nmp != 0; nmp = nmp->next)
	     if (strcmp(filename, nmp->name) == 0) break;
	  /* if it is new, output it and add it to the list */
	  if (nmp == 0) {
		script_word( "CC" );
		script_word( ss_chan_type.abbr );
		script_word( filename );
		script_word( NULL );
		nmp = new_memory( sizeof( nmlist ) );
		nmp->name = nl_strdup( filename );
		nmp->next = nml;
		nml = nmp;
	  }
	}
  }

  /* free the list of names we've built */
  while (nml != 0) {
	free_memory(nml->name);
	nmp = nml;
	nml = nml->next;
	free_memory(nmp);
  }
}

chantype ss_chan_type = {
  "sps",
  ssp_chan_delete,
  ssp_pos_create,
  ssp_pos_duplicate,
  ssp_pos_delete,
  ssp_pos_rewind,
  ssp_pos_data,
  ssp_pos_move,
  ss_channels,
  ss_report
};

int ss_channels(char *name) {
  sps_ptr ssp;
  int n_cols, i, channel_id;
  chandef *channel;
  static int n_sps_s = 0;
  char ssname[_MAX_PATH], xtitle[40];
  
  ssp = ss_open(name);
  if (ss_error(ssp)) return -1;
  n_sps_s++;

  /* get X title */
  if (ss_get_column_title(ssp, 0, xtitle) < 0 || xtitle[0] == '\0')
	strcpy(xtitle, "X");

  /* get title of each column and create a channel */
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
	  channel = channel_create(name, &ss_chan_type, channel_id,
						  xtitle, title);
	  if (channel != 0)
		chans[channel_id].ssp = ss_dup_chk(ssp);
	}
  }
  ss_close(ssp);
  return 0;
}
