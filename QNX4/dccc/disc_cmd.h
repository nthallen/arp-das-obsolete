/* 
	disc_cmd.h	- defines type of commands that dccc handles.
	Modified by Eil July 1991 for QNX.
*/

#define N_DISC_CMDS   100

/*
   The following are the command types:
   SPARE indicates that dccc does not take any action
   STRB  indicates the designated bit should be set and the strobe set also.
         These commands must be sent to dccc twice: once to start and once
         to stop.  The strobed command controller will do the necessary
         waiting.
   STEP  indicates the designated bit is a motor step command.  The line
         should be set and then immediately reset. (It is not anticipated that
         any step commands will be issued directly from command reception, since
         all of the steppable devices have drivers which keep track of step
         number.  However, I will not remove the capability of stepping the
         drives as a redundant feature.)
   SET   indicates that the bit will be set or cleared.
   NDCC  indicates that this command does not a discrete command card command,
         but is rather some other type of one argument command.
*/

#define SPARE 0
#define STRB 1
#define STEP 2
#define SET 3
/* #define NDCC 4 */
