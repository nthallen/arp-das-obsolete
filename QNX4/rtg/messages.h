/* messages.h */
#ifndef _RTG_MESSAGES_H_INCLUDED
#define _RTG_MESSAGES_H_INCLUDED

#ifndef _RTGAPI_H_INCLUDED
  #error rtgapi.h must be included before messages.h
#endif

#include <sys/types.h>

/* messages.c */
void rt_msg(pid_t pid, rtg_msg_t *msg);

#endif
