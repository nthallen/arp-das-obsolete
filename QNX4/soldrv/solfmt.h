#include <mig.h> /* for error() */

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
