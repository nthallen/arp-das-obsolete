/* channels internals
 * $Log$
 * Revision 1.1  1994/10/31  18:49:28  nort
 * Initial revision
 *
 */
#include <windows/Qwindows.h>
#include <string.h>
#include <assert.h>
#include "nortlib.h"
#include "rtg.h"

static chandef *channels = NULL;
static int n_channels = 0, n_chan_chars = 0;

/* channels_defined returns TRUE if there is at least one channel defined
 */
int channels_defined(void) { return channels != 0; }

/* channel_create(name) creates the specified channel with defaults
   Returns chandef if successful, NULL if channel already exists.
   Should I be issuing error messages?
   
   channel_create() in this incarnation is called after the
   channel has been created by the type-specific module
 */
chandef *channel_create(const char *name, chantype *type, int channel_id,
			const char *xunits, const char *yunits) {
  chandef **ccp, *cc, *nc;
  int cmp;

  if (name == 0 || *name == '\0') return NULL;
  ccp = &channels;
  for (cc = *ccp; cc != NULL; ccp = &cc->next, cc = *ccp) {
	cmp = strcmp(name, cc->name);
	if (cmp == 0) return NULL;
	if (cmp < 0) break;
  }

  /* Now create the new channel */
  nc = new_memory(sizeof(chandef));
  nc->next = cc;
  *ccp = nc;
  nc->name = nl_strdup(name);
  n_channels++;
  n_chan_chars += strlen(name);
  nc->xunits = nl_strdup(xunits);
  nc->yunits = nl_strdup(yunits);
  nc->type = type;
  nc->positions = NULL;
  nc->channel_id = channel_id;
  nc->opts = type->DfltOpts;

  return nc;
}

/* channel_delete(name); deletes the specified channel
   returns non-zero if the deletion was successful, zero if the
   channel was not found
 */
int channel_delete(const char *name) {
  chandef **ccp, *cc;
  int cmp;
  chanpos *pos;
  BaseWin *bw;

  if (name == 0 || *name == '\0') return(0);
  ccp = &channels;
  for (cc = *ccp; cc != NULL; ccp = &cc->next, cc = *ccp) {
	cmp = strcmp(name, cc->name);
	if (cmp == 0) break;
	if (cmp < 0) return(0);
  }
  if (cc == NULL) return(0);

  /* Need to delete any graphs using this channel */
  for (bw = BaseWins; bw != NULL; bw = bw->next) {
	for (;;) {
	  RtgGraph *graph;

	  for (graph = bw->graphs; graph != NULL; graph = graph->next)
		if (graph->position->channel == cc) break;
	  if (graph == 0) break;
	  graph_delete(graph);
	}
  }

  /* Need to call the type-specific delete function */
  cc->type->channel_delete(cc);
  for (pos = cc->positions; pos != NULL; pos = pos->next) {
	pos->deleted = 1;
	pos->channel = NULL;
  }

  /* Now delete the channel */
  n_channels--;
  n_chan_chars -= strlen(cc->name);
  free_memory(cc->name);
  free_memory(cc->xunits);
  free_memory(cc->yunits);
  *ccp = cc->next;
  free_memory(cc);
  return(1);
}

/* channel_props returns the properties structure for the specified channel
 * Figure out the type here!
 */
chandef *channel_props(const char *name) {
  chandef *cc;
  int cmp;

  for (cc = channels; cc != NULL; cc = cc->next) {
	cmp = strcmp(name, cc->name);
	if (cmp == 0) return(cc);
	if (cmp < 0) break;
  }
  return NULL;
}

/* channel_menu Puts up a menu of defined channels
   Does NOT provide a handler
 */
void Draw_channel_menu( const char *label, const char *title ) {
  char *buf;
  int i, bufsize;
  chandef *cd;

  if (n_channels == 0) return;  
  bufsize = n_channels+n_chan_chars;
  buf = new_memory(bufsize);
  i = 0;
  for (cd = channels; cd != NULL; cd = cd->next) {
	if (i != 0) buf[i++] = ';';
	strcpy(buf+i, cd->name);
	i += strlen(cd->name);
  }
  buf[i] = '\0';
  Menu( label, title, 1, buf, "O");
  free_memory(buf);
}

chanpos *position_create(chandef *channel) {
  int position_id;
  chanpos *pos;
  
  position_id = channel->type->position_create(channel);
  if (position_id < 0) return NULL;
  pos = new_memory(sizeof(chanpos));
  pos->next = channel->positions;
  channel->positions = pos;
  pos->channel = channel;
  pos->type = channel->type;
  pos->position_id = position_id;
  pos->at_eof = 0;
  pos->expired = 0;
  pos->reset = 0;
  pos->deleted = 0;
  return pos;
}

void position_delete(chanpos *pos) {
  chanpos **p;

  assert(pos != NULL);
  if (pos->deleted == 0) {
	/* Take it out of the chain for the channel */
	for (p = &pos->channel->positions;
		  *p != NULL && *p != pos;
		  p = &(*p)->next);
	assert(*p == pos);
	*p = pos->next;

	/* Now call the type module */
	pos->type->position_delete(pos);
  }
  free_memory(pos);
}
