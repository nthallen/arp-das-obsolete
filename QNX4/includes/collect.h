/* collect.h defines routines applications might use to
   communicate with collection.
   $Log$
 */
#ifndef _COLLECT_H_INCLUDED
#define _COLLECT_H_INCLUDED

#include <sys/types.h>

/* Message types used by collection */
struct colmsg {
  unsigned char type;
  unsigned char id;
  union {
	pid_t proxy;
	void far *pointer;
  } u;
};
#define COL_REGULATE 255
#define COL_REG_SIG 254
#define COL_TSPROXY 253
#define COL_SET_POINTER 252
#define COL_RESET_POINTER 251
#define COL_SET_PROXY 250
#define COL_RESET_PROXY 249

struct dgreg_rep {
  unsigned char reply_code;
  pid_t proxy;
};

/* COL_REGULATE creates a proxy which will send COL_REG_SIG to Collection
   and returns it in a struct dgreg_rep. If already defined, returns
   DAS_BUSY and the proxy. (Could be used by startdbr to obtain the
   proxy)

   COL_REG_SIG and COL_TSPROXY are both proxy messages and as such
   don't have any other structure.

   COL_SET_POINTER requires the id and pointer be defined. The
   pointer must have been armed for access by collection. Collection
   will convert it to a pointer it can use. The id must have been
   defined in the TMC file, else DAS_UNKN will be returned.

   COL_RESET_POINER will release the pointer associated with the
   id. If the id isn't defined or a pointer hasn't been set,
   DAS_UNKN will be returned.

   COL_SET_PROXY requires the id and proxy be defined. The proxy
   will be associated with the id as defined in the TMC file. If
   a proxy has already been set, DAS_BUSY will be returned. If the
   id has not been defined DAS_UNKN will be returned.

   COL_RESET_PROXY will disassociate the proxy from the id. If
   the id is not defined or no proxy has been set, DAS_UNKN will
   be returned. On success, the proxy value will be returned in
   the colmsg structure.
*/

/* API */
int Col_set_pointer(unsigned char id, void *pointer, unsigned flags);
int Col_reset_pointer(unsigned char id);
pid_t Col_set_proxy(unsigned char id, unsigned char msg);
int Col_reset_proxy(unsigned char id);

#endif
