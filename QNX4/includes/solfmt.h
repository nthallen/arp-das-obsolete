/* solfmt.h
 * $Log$
 * Revision 1.1  1993/02/25  15:41:21  eil
 * Initial revision
 *
 */

/*      compile.c       */
void describe(void);
void comp_waits(int j);
void compile(void);
void optimize(int mn);

/*      output.c        */
void output(char *ofile);
void read_status_addr(void);

/*      read_cmd.c      */
void read_cmd(void);
extern int cmd_set;

/*      read_d2a.c      */
void read_dtoa(void);

/*      read_mod.c      */
void init_modes(void);
void read_mode(void);

/*      read_sol.c      */
void read_sol(void);

/*      read_val.c      */
int get_change_code(int type, int dtoa_num);

/*      routines.c      */
void read_routine(void);

void read_proxy(void); /* read_pxy.c */
