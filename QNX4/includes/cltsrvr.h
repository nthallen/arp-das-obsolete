#ifndef CLTSRVR_H_INCLUDED
#define CLTSRVR_H_INCLUDED

#include <sys/types.h>
#include <sys/kernel.h>

typedef struct {
  char *name;
  int expand, global;
  int response; /* nl_response level for client requests */
  nid_t node; /* may be set during init_options */
  pid_t pid;
  int connected, disconnected; /* client status */
} Server_Def;

int CltInit( Server_Def *def );
int CltSend( Server_Def *def, void *smsg, void *rmsg, int 
				sbytes, int rbytes );
int CltSendmx( Server_Def *def, unsigned sparts, unsigned rparts,
           struct _mxfer_entry *smsg,
		   struct _mxfer_entry *rmsg );

/* Cmdctrl request (which uses cltsrvr approach) */
pid_t cc_quit_request( nid_t cc_node );

extern Server_Def PBdef;

#endif
