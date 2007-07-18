#ifndef _REPLY_H_INCLUDED
#define _REPLY_H_INCLUDED
typedef unsigned char reply_type;
#define REP_OK 0
#define REP_UNKN 1
#define REP_BUSY 2
#define REP_NO_REPLY 3
#define REP_MAX REP_NO_REPLY
#define REPLY_SZ sizeof(reply_type)
#endif
