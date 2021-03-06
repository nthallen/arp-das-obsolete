/* channels internals
 * $Log$
 * Revision 1.7  1995/01/27  20:34:07  nort
 * *** empty log message ***
 *
 * Revision 1.5  1994/12/19  16:40:42  nort
 * *** empty log message ***
 *
 * Revision 1.4  1994/12/13  16:10:26  nort
 * Realtime!
 *
 * Revision 1.3  1994/12/07  16:32:29  nort
 * *** empty log message ***
 *
 * Revision 1.2  1994/11/01  21:51:01  nort
 * *** empty log message ***
 *
 * Revision 1.1  1994/10/31  18:49:28  nort
 * Initial revision
 *
 */
#include <windows/Qwindows.h>
#include <string.h>
#include <assert.h>
#include "nortlib.h"
#include "rtg.h"

/* channels_defined returns TRUE if there is at least one channel defined
 */
int channels_defined(void) { return ChanTree_defined(CT_CHANNEL); }

/* channel_create(name) creates the specified channel with defaults
   Returns chandef if successful, NULL if channel already exists.
   Should I be issuing error messages?
   
   channel_create() in this incarnation is called after the
   channel has been created by the type-specific module
 */
chandef *channel_create(const char *name, chantype *type, int channel_id,
			  const char *xunits, const char *yunits) {
  chandef *nc;
  RtgChanNode *CN;

  if (name == 0 || *name == '\0') return NULL;
  CN = ChanTree(CT_INSERT, CT_CHANNEL, name);
  if (CN == 0) return NULL;
  assert(CN->word == 0);

  /* Now create the new channel */
  CN->u.leaf.channel = nc = new_memory(sizeof(chandef));
  nc->name = nl_strdup(name);
  nc->type = type;
  nc->positions = NULL;
  nc->channel_id = channel_id;
  nc->units.Y = dastring_init(yunits);
  nc->units.X = dastring_init(xunits);

  return nc;
}

/* channel_delete(name); deletes the specified channel
   returns non-zero if the deletion was successful, zero if the
   channel was not found
 */
int channel_delete(const char *name) {
  RtgChanNode *CN;
  chanpos *pos;
  BaseWin *bw;
  chandef *cc;

  if (name == 0 || *name == '\0') return(0);
  CN = ChanTree(CT_FIND, CT_CHANNEL, name);
  if (CN == NULL) return 0;
  assert(CN->word == 0 && CN->u.leaf.channel != 0);
  cc = CN->u.leaf.channel;

  /* Terminate any outstanding properties windows */
  PropCancel(cc->name, "CP", "P");
  /* chanprop_delete(cc); */

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
  dastring_update(&cc->name, NULL);
  dastring_update(&cc->units.X, NULL);
  dastring_update(&cc->units.Y, NULL);
  free_memory(cc);
  CN->u.leaf.channel = NULL;
  ChanTree(CT_DELETE, CT_CHANNEL, name);
  return(1);
}

/* channel_props returns the properties structure for the specified channel
 * Figure out the type here!
 */
chandef *channel_props(const char *name) {
  RtgChanNode *CN;
  
  CN = ChanTree(CT_FIND, CT_CHANNEL, name);
  if (CN != 0 && CN->word == 0)
	return CN->u.leaf.channel;
  else return NULL;
}

static chanpos *new_position(chandef *channel, int position_id) {
  chanpos *pos;
  
  if (position_id < 0)
	nl_error( 4, "Invalid position in new_position" );
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

chanpos *position_create(chandef *channel) {
  int position_id;
  
  position_id = channel->type->position_create(channel);
  return new_position(channel, position_id);
}

chanpos *position_duplicate(chanpos *oldpos) {
  int position_id;
  
  position_id = oldpos->type->position_duplicate(oldpos);
  return new_position(oldpos->channel, position_id);
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
