/* dummy.c A template for channel type creation */
#include <stdlib.h>
#include "rtg.h"
#include "nortlib.h"

#define MIN_ALLOC 8
typedef struct {
  int in_use;
} chan_info;

static chan_info *chans = NULL;
static int n_chans = 0;

typedef struct {
  int in_use;
} pos_info;

static pos_info *poses = NULL;
static int n_poses = 0;

static void dum_chan_delete(chandef *channel) {
  chans[channel->channel_id].in_use = 0;
}

static int dum_pos_create(chandef *channel) {
  int pos_id, i;

  for (pos_id = 0; pos_id < n_poses; pos_id++)
	if (poses[pos_id].in_use == 0) break;
  if (pos_id == n_poses) {
	n_poses = (n_poses > 0) ? n_poses * 2 : MIN_ALLOC;
	poses = realloc(poses, sizeof(chan_info)*n_poses);
	if (poses == 0) nl_error(3, "Memory allocation failure in dum_pos_create");
	for (i = pos_id; i < n_poses; i++) poses[i].in_use = 0;
  }
  poses[pos_id].in_use = 1;
  return pos_id;
}

static int dum_pos_duplicate(chanpos *position) {
  int pos_id;
  
  pos_id = dum_pos_create(position->channel);
  return pos_id;
}

static void dum_pos_delete(chanpos *position) {
  poses[position->position_id].in_use = 0;
}

static int dum_pos_rewind(chanpos *position) {
  position->at_eof = 1;
  return 1;
}

static int dum_pos_data(chanpos *position, double *X, double *Y) {
  position->at_eof = 1;
  return 0;
}

static int dum_pos_move(chanpos *position, long int index) {
  return 0;
}

static chantype dummy = {
  dum_chan_delete,
  dum_pos_create,
  dum_pos_duplicate,
  dum_pos_delete,
  dum_pos_rewind,
  dum_pos_data,
  dum_pos_move
};

void dummy_channel_create(const char *name) {
  int channel_id, i;

  for (channel_id = 0; channel_id < n_chans; channel_id++)
	if (chans[channel_id].in_use == 0) break;
  if (channel_id == n_chans) {
	n_chans = (n_chans > 0) ? n_chans * 2 : MIN_ALLOC;
	chans = realloc(chans, sizeof(chan_info)*n_chans);
	if (chans == 0)
	  nl_error(3, "Memory allocation failure in dummy_channel_create");
	for (i = channel_id; i < n_chans; i++) chans[i].in_use = 0;
  }
  if (channel_create(name, &dummy, channel_id) != 0) {
	chans[channel_id].in_use = 1;
  } /* else undo what you did */
}
