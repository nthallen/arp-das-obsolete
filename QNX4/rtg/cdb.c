/* cdb.c Defines functions for circular data buffers
*/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "nortlib.h"
#include "rtg.h"
#include "rtgapi.h"
#include "messages.h"
#include "ssp.h" /* for is_number */

typedef struct cdb_str {
  cdb_data_t (*data)[2];
  cdb_data_t last_x;
  cdb_index_t n_pts;
  cdb_index_t first, last;
  chandef *channel;
  unsigned char published:1;
} cdb_t;

typedef struct cdb_pos_str {
  struct cdb_str *cdb;
  cdb_index_t index;
} cdb_pos_t;

#define MIN_ALLOC 8
static cdb_t **chans = NULL;
static int n_chans = 0;
static cdb_pos_t *poses = NULL;
static int n_poses = 0;

/* Circular data buffer semantics:
   last >= n_pts      ==> EMPTY
   else data[first] is first data point
        data[last] points to empty space after most recently entered data point
   first should not == last except in the empty case.
   
   Position semantics
     if buffer is empty, all index values are invalid
	 if buffer is non-empty, an index equal to n_pts is invalid
	 and shouldn't occur (assert()). All other indices can be
	 assumed to lie in the range of valid data, although it is
	 difficult to verify due to the circular nature. In any
	 event, index == cdb->last indicates EOF.
*/

/* This is the largest possible request, though not necessarily the
   largest actual buffer, since system resource limitations may apply
*/
static cdb_index_t cdb_max_size = (cdb_index_t)(32768L/sizeof(cdb_data_t));

static cdb_t *cdb_create(cdb_index_t size) {
  cdb_t *cdb;
  
  if (size == 0 || size >= cdb_max_size)
	return NULL;
  cdb = new_memory(sizeof(cdb_t));
  cdb->data = new_memory(size * 2 * sizeof(cdb_data_t));
  cdb->n_pts = size;
  cdb->first = cdb->last = size; /* indicates empty */
  cdb->channel = NULL;
  cdb->published = 0;
  return cdb;
}

int cdb_resize(int channel_id, cdb_index_t newsize) {
  cdb_t *cdb;
  cdb_data_t *newdata;
  assert(channel_id >= 0 && channel_id < n_chans);
  cdb = chans[channel_id];
  assert(cdb != 0);
  if ( newsize >= cdb_max_size )
	newsize = cdb_max_size - 1;
  if ( cdb->n_pts >= newsize ) return 1;
  newdata = realloc( cdb->data, newsize*2*sizeof(cdb_data_t) );
  if ( newdata == 0 ) return 0;
  cdb->data = newdata;
  if ( cdb->first == cdb->n_pts ||
	   ( cdb->first < cdb->n_pts && cdb->first > cdb->last ) )
	cdb->first = cdb->last = newsize;
  cdb->n_pts = newsize;
  if ( cdb->first == cdb->n_pts )
	cdb_new_point( channel_id, non_number, 0. );  /* reset positions */
  return 1;
}

/* 
   Modified to not free the cdb structure itself if this cdb
   has been published. The data buffer is freed, though.
*/
static cdb_t *cdb_delete(cdb_t *cdb) {
  if (cdb != NULL) {
	if (cdb->data != 0) {
	  free_memory(cdb->data);
	  cdb->data = NULL;
	}
	if (cdb->published) {
	  cdb->n_pts = 0;
	  cdb->channel = NULL;
	} else {
	  free_memory(cdb);
	  cdb = NULL;
	}
  }
  return cdb;
}

static void cdb_chan_delete(chandef *channel) {
  int chan_id;
  
  chan_id = channel->channel_id;
  assert(chan_id >= 0 && chan_id < n_chans && chans[chan_id] != 0);
  chans[chan_id] = cdb_delete(chans[chan_id]);
}

/* Returns 1 if the point is successfully enqueued, 0 if the
   channel has been deleted
*/
int cdb_new_point(int channel_id, cdb_data_t X, cdb_data_t Y) {
  chanpos *p;
  cdb_t *cdb;
  cdb_pos_t *cdbp;
  cdb_data_t *data;
  cdb_index_t first, next;

  assert(channel_id >= 0 && channel_id < n_chans);
  cdb = chans[channel_id];
  assert(cdb != 0);
  if (cdb->n_pts == 0) return 0;

  /* Verify monotonicity */
  if ((!is_number(X)) ||
	  (cdb->last < cdb->n_pts && cdb->last_x >= X)) {
	cdb->last = cdb->first = cdb->n_pts;
	for (p = cdb->channel->positions; p != NULL; p = p->next) {
	  p->reset = 1;
	  poses[p->position_id].index = cdb->n_pts;
	}
  }
  if (! is_number(X) ) return 1;
  
  if (cdb->last >= cdb->n_pts) {
	/* empty */
	cdb->first = cdb->last = 0;
  }
  data = &cdb->data[cdb->last][0];
  cdb->last_x = data[0] = X;
  data[1] = Y;
  if (++cdb->last >= cdb->n_pts)
	cdb->last = 0;
  if (cdb->last == cdb->first) {
	/* full */
	first = cdb->first;
	next = first+1;
	if (next >= cdb->n_pts)
	  next = 0;
	cdb->first = next;
  }

  /* Update all the positions */
  for (p = cdb->channel->positions; p != NULL; p = p->next) {
	p->at_eof = 0;
	cdbp = &poses[p->position_id];
	if (cdbp->index == cdb->last) {
	  p->expired = 1;
	  cdbp->index = cdb->first;
	} else if (cdbp->index >= cdb->n_pts)
	  cdbp->index = cdb->first;
  }
  return 1;
}

static int cdb_pos_create(chandef *channel) {
  int pos_id, i;
  int cid;

  assert(channel != 0);
  cid = channel->channel_id;
  assert(cid >= 0 && cid < n_chans && chans[cid] != 0);
  for (pos_id = 0; pos_id < n_poses; pos_id++)
	if (poses[pos_id].cdb == 0) break;
  if (pos_id == n_poses) {
	n_poses = (n_poses > 0) ? n_poses * 2 : MIN_ALLOC;
	poses = realloc(poses, sizeof(cdb_pos_t)*n_poses);
	if (poses == 0)
	  nl_error(3, "Memory allocation failure in cdb_pos_create");
	for (i = pos_id; i < n_poses; i++)
	  poses[i].cdb = NULL;
  }
  poses[pos_id].cdb = chans[cid];
  /* Might need some kind of initialization here... */
  /* May be able to count on the graph to be redrawn, hence rewound
     before any action is taken...*/
  poses[pos_id].index = chans[cid]->first;
  return pos_id;
}

static int cdb_pos_duplicate(chanpos *position) {
  int pos_id;
  
  pos_id = cdb_pos_create(position->channel);
  poses[pos_id].index = poses[position->position_id].index;
  return pos_id;
}

static void cdb_pos_delete(chanpos *position) {
  int pos_id;
  
  assert(position != 0);
  pos_id = position->position_id;
  assert(pos_id >= 0 && pos_id < n_poses && poses[pos_id].cdb != 0);
  poses[pos_id].cdb = NULL;
}

/* returns TRUE if there is data in the cdb */
static int cdb_pos_rewind(chanpos *p) {
  cdb_t *cdb;
  int pos_id;

  assert(p != 0);
  pos_id = p->position_id;
  assert(pos_id >= 0 && pos_id < n_poses);
  cdb = poses[pos_id].cdb;
  assert(cdb != 0);

  if (cdb->last >= cdb->n_pts) {
	/* empty */
	poses[pos_id].index = cdb->n_pts;
	p->at_eof = 1;
  } else {
	poses[pos_id].index = cdb->first;
	p->at_eof = 0;
  }
  p->reset = 0;
  p->expired = 0;
  return (! p->at_eof);
}

/* Return 1 if data is provided. Advance index by one */
static int cdb_pos_data(chanpos *p, double *X, double *Y) {
  cdb_t *cdb;
  int pos_id;
  cdb_index_t idx;
  cdb_data_t *data;

  assert(p != 0);
  pos_id = p->position_id;
  assert(pos_id >= 0 && pos_id < n_poses);
  cdb = poses[pos_id].cdb;
  assert(cdb != 0);
  idx = poses[pos_id].index;
  if (cdb->last >= cdb->n_pts || idx == cdb->last) return 0;
  assert(idx < cdb->n_pts);
  data = &cdb->data[idx][0];
  *X = data[0];
  *Y = data[1];
  if (++idx == cdb->n_pts)
	idx = 0;
  poses[pos_id].index = idx;
  if (idx == cdb->last)
	p->at_eof = 1;
  return 1;
}

static int cdb_pos_move(chanpos *p, long int index) {
  cdb_t *cdb;
  int pos_id;
  long int idx, lf, ll;

  assert(p != 0);
  pos_id = p->position_id;
  assert(pos_id >= 0 && pos_id < n_poses);
  cdb = poses[pos_id].cdb;
  assert(cdb != 0);
  idx = poses[pos_id].index;
  if (cdb->last >= cdb->n_pts) {
	p->at_eof = 1;
	return 0;
  }
  lf = cdb->first;
  ll = cdb->last;
  if (ll < lf) {
	ll += cdb->n_pts;
	assert(idx <= cdb->last || idx >= cdb->first);
	if (idx < cdb->first) idx += cdb->n_pts;
  } else {
	assert(idx >= cdb->first && idx <= cdb->last);
  }
  if (index >= ll - idx) {
	idx = ll;
	p->at_eof = 1;
  } else if (index <= lf - idx) {
	idx = lf;
	p->at_eof = 0;
  } else {
	idx += index;
	p->at_eof = 0;
  }
  if (idx >= cdb->n_pts) {
	idx -= cdb->n_pts;
  }
  assert(idx >= 0 && idx < cdb->n_pts);
  poses[pos_id].index = idx;
  return 0;
}

/* outputs script commands to recreate the current cdb channels */
static void cdb_report(void) {
  int channel_id;
  
  for (channel_id = 0; channel_id < n_chans; channel_id++ ) {
	if ( chans[ channel_id ] != 0 ) {
	  char *chname, *p;
	  
	  chname = nl_strdup( chans[ channel_id ]->channel->name );
	  if (p = strrchr( chname, '/' ) )
		*p = '\0';
	  /* assuming the rest of the string is "rtg" */
	  script_word( "CC" );
	  script_word( cdb_type.abbr );
	  script_word( chname );
	  script_word( NULL );
	  free_memory( chname );
	}
  }
}

chantype cdb_type = {
  "rtg",
  cdb_chan_delete,
  cdb_pos_create,
  cdb_pos_duplicate,
  cdb_pos_delete,
  cdb_pos_rewind,
  cdb_pos_data,
  cdb_pos_move,
  cdb_channel_create,
  cdb_report
};

/* Returns the channel ID if successful, -1 otherwise */
int cdb_channel_create(const char *name) {
  int channel_id, i;
  chandef *channel;

  if ( strlen(name) >= _MAX_PATH ) {
	nl_error( 2, "Channel name too long: %s", name );
	return -1;
  }
  for (channel_id = 0; channel_id < n_chans; channel_id++)
	if (chans[channel_id] == 0) break;
  if (channel_id == n_chans) {
	n_chans = (n_chans > 0) ? n_chans * 2 : MIN_ALLOC;
	chans = realloc(chans, sizeof(cdb_t *)*n_chans);
	if (chans == 0)
	  nl_error(3, "Memory allocation failure in cdb_channel_create");
	for (i = channel_id; i < n_chans; i++) chans[i] = NULL;
  }
  { char chname[_MAX_PATH+30];
	cdb_t *cdb;
	
	sprintf(chname, "%s/rtg", name);
	channel = channel_create(chname, &cdb_type, channel_id, "Time", name);
	if (channel == 0) {
	  /* Maybe it already exists? */
	  channel = channel_props(chname);
	  if (channel != 0 && channel->type == &cdb_type)
		return channel->channel_id;
	  else return -1;
	}
	cdb = chans[channel_id] = cdb_create(3000);
	if (cdb != 0) {
	  cdb->channel = channel;
	  cdb->published = 1;
	} else {
	  channel_delete(chname);
	  return -1;
	}
  }
  return channel_id;
}
