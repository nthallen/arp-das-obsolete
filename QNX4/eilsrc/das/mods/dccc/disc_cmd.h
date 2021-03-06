/* 
	disc_cmd.h	- defines type of commands that dccc handles.
	Modified by Eil July 1991 for QNX.
*/

#define N_DISC_CMDS   100

/*
   The following are the command types:
   SPARE indicates that dccc does not take any action
   STRB  indicates the designated bits (mask) for a port should be set and
         the strobe set also.
         These commands must be sent to dccc twice: once to start and once
         to stop.  The strobed command controller will do the necessary
         waiting.
   STEP  indicates the designated bits (mask) is a motor step command.  The
         lines should be set and then immediately reset. (It is not
         anticipated that any step commands will be issued directly from
         command reception, since all of the steppable devices have drivers
         which keep track of step number.  However, I will not remove the
         capability of stepping the drives as a redundant feature.)
   SET   indicates that bits of a port will be set or cleared. If mask=0, the
         bits are designated by given value. If mask != 0, then bits (mask) are
         cleared if value == 0, else they are set.
   SELECT indicates that select bits (mask) of a port are set to given value.
         Selected bits are re-inverted.
   NDCC  indicates that this command does not a discrete command card command,
         but is rather some other type of one argument command.
*/

#define SPARE 0
#define STRB 1
#define STEP 2
#define SET 3
#define SELECT 4
/* #define NDCC 4 */
