/* cdb.c Defines functions for circular data buffers
*/
#include <assert.h>
#include "nortlib.h"
#include "rtg.h"
#include "cdb.h"

/* Circular data buffer semantics:
   last >= n_pts      ==> EMPTY
   else data[first] is first data point
        data[last] points to empty space after most recently entered data point
   first should not == last except in the empty case.
*/

/* This is the largest possible request, though not necessarily the
   largest actual buffer, since system resource limitations may apply
*/
cdb_index_t cdb_max_size = (cdb_index_t)(32768L/sizeof(cdb_data_t));

cdb_t *cdb_create(cdb_index_t size) {
  cdb_t *cdb;
  
  if (size == 0 || size >= cdb_max_size)
	return NULL;
  cdb = new_memory(sizeof(cdb_t));
  cdb->data = new_memory(size * 2 * sizeof(cdb_data_t));
  cdb->n_pts = size;
  cdb->first = cdb->last = size; /* indicates empty */
  cdb->positions = NULL;
  return cdb;
}

/* Returns NULL to clarify support code which clears the pointer after deletion */
cdb_t *cdb_delete(cdb_t *cdb) {
  if (cdb != NULL) {
	if (cdb->data != NULL)
	  free_memory(cdb->data);
	free_memory(cdb);
  }
  return NULL;
}

/* int cdb_resize(int newsize); */

void cdb_new_point(cdb_t *cdb, cdb_data_t X, cdb_data_t Y) {
  cdb_pos_t *p;
  cdb_data_t *data;
  cdb_index_t first, next;

  /* Verify monotonicity */
  if (cdb->last < cdb->n_pts && cdb->data[cdb->last][0] >= X) {
	cdb->last = cdb->first = cdb->n_pts;
	for (p = cdb->positions; p != NULL; p = p->next) {
	  p->reset = 1;
	  p->index = cdb->n_pts;
	}
  }
  
  if (cdb->last >= cdb->n_pts) {
	/* empty */
	cdb->first = cdb->last = 0;
  }
  data = &cdb->data[cdb->last][0];
  data[0] = X;
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
	for (p = cdb->positions; p != NULL; p = p->next) {
	  if (p->index == first) {
		p->expired = 1;
		p->index = next;
	  }
	}
  }
  for (p = cdb->positions; p != NULL; p = p->next) {
	p->at_eof = 0;
	if (p->index >= cdb->n_pts)
	  p->index = cdb->first;
  }
}

cdb_pos_t *cdb_pos_create(cdb_t *cdb) {
  cdb_pos_t *p;
  
  p = new_memory(sizeof(cdb_pos_t));
  p->next = cdb->positions;
  cdb->positions = p;
  p->cdb = cdb;
  cdb_pos_rewind(p);
  return p;
}

void cdb_pos_delete(cdb_pos_t *pos) {
  cdb_pos_t **pp;
  
  assert(pos != NULL && pos->cdb != NULL && pos->cdb->positions != NULL);
  for (pp = &pos->cdb->positions; *pp != NULL && *pp != pos; pp = &(*pp)->next) ;
  assert(*pp != NULL);
  *pp = (*pp)->next;
  free_memory(pos);
}

/* returns TRUE if there is data in the cdb */
int cdb_pos_rewind(cdb_pos_t *p) {
  cdb_t *cdb;

  assert(p != NULL && p->cdb != NULL);
  cdb = p->cdb;
  if (cdb->last >= cdb->n_pts) {
	/* empty */
	p->index = cdb->n_pts;
	p->at_eof = 1;
  } else {
	p->index = cdb->first;
	p->at_eof = 0;
  }
  p->reset = 0;
  p->expired = 0;
  return (! p->at_eof);
}

cdb_index_t cdb_pos_npts(cdb_pos_t *p, cdb_index_t *before) {
  cdb_t *cdb;
  cdb_index_t n_pts, after;
  
  assert(p != NULL && p->cdb != NULL);
  cdb = p->cdb;
  n_pts = cdb->n_pts;
  if (cdb->last >= n_pts) {
	if (before != 0) *before = 0;
	return 0;
  }
  if (p->index >= n_pts) cdb_pos_rewind(p);

  if (before != 0) {
	if (p->index >= cdb->first) *before = n_pts;
	else *before = 0;
	*before += p->index - cdb->first;
  }

  if (p->index <= cdb->last) after = 0;
  else after = n_pts;
  after += cdb->last - p->index;
  
  return after;
}

int cdb_pos_data(cdb_pos_t *p, int dir, cdb_index_t index, cdb_data_t *X, cdb_data_t *Y) {
  cdb_t *cdb;
  cdb_index_t before, after, idx, n_pts;
  cdb_data_t *data;

  after = cdb_pos_npts(p, &before);
  cdb = p->cdb;
  idx = p->index;
  n_pts = cdb->n_pts;
  if (index == 0) {
	if (after == 0) return 0;
	if (++p->index >= n_pts) p->index = 0;
	if (p->index == cdb->last)
	  p->at_eof = 1;
  } else if (dir != 0) { /* after */
	if (index >= after)
	  return 0;
	idx += index;
	if (idx > n_pts)
	  idx -= n_pts;
  } else { /* before */
	if (index > before)
	  return 0;
	else if (index > idx)
	  idx += n_pts;
	idx -= index;
  }
  data = cdb->data[idx];
  *X = data[0];
  *Y = data[1];
  return 1;
}

#ifdef NOTREALLY
int cdb_pos_move(cdb_pos_t *p, int dir, cdb_index_t where) {
  return 0;
}
#endif

