/* ssp.h defines everything a programmer needs to know to access a
   spreadsheet.
 * $Log$
 * Revision 1.3  1992/08/20  21:01:23  nort
 * extern dnan ==> extern union dnan
 *
 * Revision 1.2  1992/08/12  19:39:37  nort
 * Added and removed snafuerr.h.
 * Incorporated relation.h
 *
 * Revision 1.1  1992/08/12  19:31:42  nort
 * Initial revision
 *
 * $Id$
   Written June 20, 1988
   Made ANSI September 21, 1989
     Added conditional inclusion of stdio.h
*/
#ifndef _SSP_H
#define _SSP_H

typedef int sps_ptr;
#define ss_error(ssp) ((ssp) < 0)
#define SPT_NON_MONOTONIC 0
#define SPT_INCREASING 1
#define SPT_DECREASING 2

void ck_report(int depth, char *ctrl, ...);
int is_infinity(double);
int is_number(double);
char *nan_text(int type);
int q_prompt(char *ctrl, ...);
void ss_autoflush(sps_ptr ssp, int on);
int ss_check(sps_ptr ssp, int readit);
sps_ptr ss_create(char *, int, int, int);
int ss_close(sps_ptr);
void ss_close_all(void);
char *ss_convert(char *, char *, char *, double);
int ss_delete_row(sps_ptr);
sps_ptr ss_dup(sps_ptr);
sps_ptr ss_open(char *);
int ss_type(sps_ptr);
int ss_width(sps_ptr);
int ss_flush(sps_ptr);
int ss_name(sps_ptr, char *);
int ss_jump_row(sps_ptr, long int);
int ss_next_row(sps_ptr);
int ss_insert_value(sps_ptr, double, double);
int ss_insert_row(sps_ptr);
int ss_find_value(sps_ptr, double);
int ss_get(sps_ptr, int, double *);
int ss_get_column_title(sps_ptr, int, char *);
int ss_get_column_format(sps_ptr, int, char *);
int ss_get_column_lower(sps_ptr, int, double *);
int ss_get_column_upper(sps_ptr, int, double *);
long int ss_row(sps_ptr);
int ss_save(sps_ptr);
int ss_search(sps_ptr, int, long int, long int, int, double, double);
int ss_set(sps_ptr, int, double);
int ss_set_column(sps_ptr ssp, int c, char *format, char *title);
int ss_set_column_title(sps_ptr, int, char *);
int ss_set_column_format(sps_ptr, int, char *);
int ss_set_column_lower(sps_ptr, int, double);
int ss_set_column_upper(sps_ptr, int, double);
void ss_mem_salvage(void);
void ss_parse_format(char *, char *);
void ss_salvage(void);
extern int (*term_early)(char *fmt, ...);
int _term_early(char *fmt, ...);
union dnan {
  unsigned short w[4];
  double d;
};
extern union dnan _nan_;
#define non_number _nan_.d

/* relations.h defines the relations used in searches.
   Written May 30, 1987 at Ames Research Center, Moffett Field, CA
*/
#define REL_EQ 0
#define REL_NE 1
#define REL_GT 2
#define REL_GE 3
#define REL_LT 4
#define REL_LE 5

/* routines used internally in ssp.lib */
int ssp_check(sps_ptr);
void ssp_clear_refs(sps_ptr ssp);
int update(sps_ptr ssp);

#endif
