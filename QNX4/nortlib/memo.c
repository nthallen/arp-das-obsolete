#include <sys/name.h>
#include <sys/kernel.h>
#include "memo.h"
#include "reply.h"
#include "nortlib.h"

/* Returns non-zero on success. Looks at nl_response, but won't
   actually die on failures. */
int memo_shutdown( nid_t node ) {
  reply_type rv;
  int my_response;
  pid_t who;
  
  my_response = nl_response > 2 ? 2 : nl_response;
  rv = MEMO_DEATH_HDR;
  if ( (who = qnx_name_locate(node,nl_make_name(MEMO,0),0,0))!=-1) {
	if ( Send(who, &rv, &rv,
			sizeof(reply_type), sizeof(reply_type)) == -1 )	{
	  if ( my_response )
		nl_error( my_response,
		  "error sending to %s: task %d", MEMO, who );
	  return 0;
	}
	if ( rv != REP_OK ) {
	  if ( my_response )
		nl_error( my_response,
		  "bad response from %s: task %d", MEMO, who );
	  return 0;
	}
  } else {
	nl_error( my_response,
			"Can't find %s on node %d", MEMO, node );
	return 0;
  }
  return 1;
}
/*
=Name memo_shutdown(): Request memo termination
=Subject Shutdown
=Synopsis
#include "nortlib.h"
int memo_shutdown( nid_t node );

=Description

  memo_shutdown() requests that the instance of memo for the
  current Experiment running on the specified node terminate.

=Returns

  On success, memo_shutdown() returns a non-zero value. On
  failure, it will observe the =nl_response= codes, but it will
  not take any action higher then 2 (Error). In other words,
  memo_shutdown() will always return. On error, a message may be
  issued via =nl_error=(), depending on the value of
  =nl_response=, and a zero value will be returned.

=End
*/
