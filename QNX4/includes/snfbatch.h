/* batch.h defines various things pertinent to batch operations.
   Written June 13, 1987
   Memory function defs removed to memory.h December 14, 1989
*/

/* input_mode indicates in which mode input is coming in.
   IM_BATCH_0 is quit on error.
   IM_BATCH_1 is query on error.
   IM_BATCH_2 is charge right on ahead.
   IM_BATCH_3 is an internal mode used by the plotter for reading in .plt
	      files.  On error, the current file is closed and the error
	      is handled based on the saved batch mode.
*/
extern int input_mode;
#define IM_INTERACTIVE 0
#define IM_BATCH_0 1
#define IM_BATCH_1 2
#define IM_BATCH_2 3
#define IM_BATCH_3 4

void init_strvars(void);
void batch_args(int argc, char **argv);
int read_batch(char *, int);
void suspend_batch(void);
void quit_batch(void);
void resume_batch(void);
int bgetch(void);
void bungetch(int c);
int mgetch(void);
int kgetch(void);
int ekgetch(void);
void cmderr(char *, ...);
void define_var(char *, char *);
char *get_var(char *);
void shift(void);
void idle(void);
int snfkbhit(void);
int snfgetc(void);

#ifndef _SNF_INTFH
#define _SNF_INTFH
void n_prompt(char *text,...);
int q_prompt(char *text,...);
void l_prompt(char *prompt, char *input, int size);
#endif
