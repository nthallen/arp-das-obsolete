/* dfs_info.c simply defines variable dbr_info in one place so
   it may easily be included in the library whether needed for
   DG or DC purposes.
*/

#include "dfs.h"
dbr_info_type dbr_info = {0};
dfs_msg_type *dfs_msg = 0;
int dfs_msg_size = 0;
msg_hdr_type DFS_default_hdr = DEATH;
int dc_tok = 0;
int dg_tok = 0;
pid_t dfs_who = 0;

