/* collect.h defines routines applications might use to
   communicate with collection.
   $Log$
 * Revision 1.1  1993/01/09  15:50:41  nort
 * Initial revision
 *
 */
#ifndef _COLLECT_H_INCLUDED
#define _COLLECT_H_INCLUDED

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1);

/* Message types used by collection */
#define MAX_COLMSG_SIZE 100
struct colmsg {
  unsigned char type;
  unsigned char id;
  union {
	pid_t proxy;
	void far *pointer;
	unsigned char name[MAX_COLMSG_SIZE-2];
	struct {
	  unsigned short size;
	  unsigned char id;
	  unsigned char data[MAX_COLMSG_SIZE-5];
	} data;
  } u;
};
#define COL_REGULATE 255
#define COL_REG_SIG 254
#define COL_TSPROXY 253
#define COL_SET_POINTER 252
#define COL_RESET_POINTER 251
#define COL_SET_PROXY 250
#define COL_RESET_PROXY 249
#define COL_SEND 248
/* id codes corresponding to type COL_SEND */
#define COL_SEND_INIT 'I'
#define COL_SEND_SEND 'S'
#define COL_SEND_RESET 'R'

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
   
   COL_SEND is used for data which is sent to collection via
   standard QNX IPC. The sub-types are placed in the id field
   of the colmsg structure and include functions to initialize,
   use and reset a SEND connection.
   
   COL_SEND/COL_SEND_INIT:  The initialization uses the
   ASCIIZ name for the particular data as specified in the
   'TM "Receive" name 0;' statement. Collection will locate the
   specified name and return the pre-determined ID number. Possible
   return values in the type field:
     DAS_OK   id field of colmsg is the id for future correspondence.
	 DAS_UNKN the name wasn't found. Don't call back!
	 DAS_BUSY the name is in use by another program.

   COL_SEND/COL_SEND_SEND: Uses the data substructure of colmsg.
   The id established with the _INIT col goes in the id field.
   The size as determined by the sender is included with the
   message. It is compared to the size we like, and the smaller
   value is used for the actual data transfer.
   
   COL_SEND/COL_SEND_RESET: Uses the data substructure, but only
   the id field is used.
*/

/* API */
int Col_set_pointer(unsigned char id, void *pointer, unsigned flags);
int Col_reset_pointer(unsigned char id);
pid_t Col_set_proxy(unsigned char id, unsigned char msg);
int Col_reset_proxy(unsigned char id);

#include <sys/sendmx.h>

typedef struct {
  struct _mxfer_entry mx[3];
  struct colmsg *hdr;
  unsigned char rv;
} send_id_struct, *send_id;

send_id Col_send_init(const char *name, void *data, unsigned short size);
/* Col_send_init uses dynamic allocation to generate structures */
int Col_send(send_id sender);
int Col_send_reset(send_id sender);

#pragma pack();

#ifdef __cplusplus
};
#endif

#endif
