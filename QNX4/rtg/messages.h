/* messages.h */
#ifndef _RTG_MESSAGES_H_INCLUDED
#define _RTG_MESSAGES_H_INCLUDED

#ifndef _RTGAPI_H_INCLUDED
  #error rtgapi.h must be included before messages.h
#endif

#include <sys/types.h>

/* messages.c */
void rt_msg(pid_t pid, rtg_msg_t *msg);

/* cdb.c */
typedef double cdb_data_t;
typedef unsigned short cdb_index_t;
int cdb_channel_create(const char *name);
int cdb_new_point(int channel_id, cdb_data_t X, cdb_data_t Y);

#endif
