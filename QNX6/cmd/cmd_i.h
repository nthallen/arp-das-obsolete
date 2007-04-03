#ifndef CMD_I_H
#define CMD_I_H

/* These defs must preceed the following includes: */
struct ocb;
#define IOFUNC_OCB_T struct ocb
#define IOFUNC_ATTR_T struct ioattr_s
#define THREAD_POOL_PARAM_T dispatch_context_t

#include <sys/iofunc.h>
#include <sys/dispatch.h>

#define MAX_COMMAND_OUT 160 // Maximum command message output length

typedef struct command_out_s {
  struct command_out_s *next;
  int ref_count;
  char command[MAX_COMMAND_OUT];
} command_out_t;

typedef struct ocb {
  iofunc_ocb_t hdr;
  command_out_t *next_command;
  struct ocb *next_ocb; // for blocked list
  int rcvid;
} ocb_t;

/* IOFUNC_ATTR_T extended to provide blocked-ocb-list
   for each mountpoint */
typedef struct ioattr_s {
  iofunc_attr_t attr;
  ocb_t *blocked;
  command_out_t *commands;
} ioattr_t;

extern command_out_t *new_command(void);
extern void free_command( command_out_t *cmd );
extern IOFUNC_ATTR_T *setup_rdr( char *node );
// list of mountpoints/command types

#endif
