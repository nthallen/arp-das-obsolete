/* bomem.h definitions for bomem rtg program
 * $Log$
 * Revision 1.3  2005/06/23 15:53:37  nort
 * Source as it stands.
 *
 * Revision 1.2  1994/11/28  17:31:04  nort
 * Rough separation of acquisition and windows
 *
 * Revision 1.1  1994/11/23  21:52:13  nort
 * Initial revision
 *
 */
 
#define SRCDIR "/usr/local/src/das/rtg/"
#define BOMEM_MSG_MAX 50

/* bomem.c */
void Initialize_DSP(void);
void main(int argc, char **argv);
extern int log_data, collect_spec, spec_collected;
extern int sequence, n_scans;
void acquire_data(void);

/* server.c */
void server_command(pid_t from, char *cmd);
void server_loop(void);
