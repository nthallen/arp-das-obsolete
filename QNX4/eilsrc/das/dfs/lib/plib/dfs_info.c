/* dbr_info.c simply defines variable dbr_info in one place so
   it may easily be included in the library whether needed for
   DG or DC purposes.
   Written 5/24/91
   Modified 5/13/92 by Eil.
*/

#include <signal.h>
#include <dfs.h>
dbr_info_type dbr_info = {0};
dfs_msg_type *dfs_msg = 0;
int dfs_msg_size = 0;
int msg_size = 0;
int my_ipc = 0;
sigset_t sigs = 0L;
