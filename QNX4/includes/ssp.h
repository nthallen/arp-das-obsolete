/* ssp.h defines everything a programmer needs to know to access a
   spreadsheet.
 * $Log$
 * Revision 1.1  1992/08/11  19:44:37  nort
 * DOS Revision
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
  unsigned long w[4];
  double d;
};
extern dnan _nan_;
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

/* snafuerr.h defines error codes placed in external variable snafu.
*/
#ifndef _SNAFUERRH
#define _SNAFUERRH 1

extern int snafu;

#define SFU_OK   0                      /* Successful Return            */
#define SFU_GENERAL             (-1)    /* General, unspecified error   */
#define SFU_FOPEN_FAILED        (-2)    /* Same as FE_COF               */
#define SFU_SYNTAX_ERROR        (-3)    /* Same as FE_SYNT              */
#define SFU_NOT_IMPLEMENTED     (-4)
#define SFU_ILLEGAL_NAME        (-5)
#define SFU_SS_OPEN             (-6)
#define SFU_BAD_MODE            (-7)
#define SFU_NO_CLOBBER          (-8)
#define SFU_BAD_DEALL           (-9)
#define SFU_NO_INFO             (-10)
#define SFU_NOT_SPREADSHEET     (-11)
#define SFU_WRONG_VERSION       (-12)
#define SFU_BLOCK_OOR           (-13)
#define SFU_SEEK_ERROR          (-14)
#define SFU_READ_ERROR          (-15)
#define SFU_BAD_BLOCK_TYPE      (-16)
#define SFU_BAD_COLUMNS         (-17)
#define SFU_ROW_OOR             (-18)
#define SFU_BAD_SPT_TYPE        (-19)
#define SFU_VALUE_NOT_FOUND     (-20)
#define SFU_BLOCK_IN_USE        (-21)
#define SFU_LOST_IT             (-22)
#define SFU_COLUMN_OOR          (-23)
#define SFU_WINDOW              (-24)
#define SFU_VALUE_FOUND         (-25)
#define SFU_BAD_RELATION        (-26)
#define SFU_NOT_A_NUMBER        (-27)
#define SFU_BAD_MNEMONIC        (-28)
#define SFU_NO_DATA_REQ         (-29)
#define SFU_BAD_TIMES           (-30)
#define SFU_BAD_PATTERN         (-31)
#define SFU_NO_SUCH_FILE        (-32)
#define SFU_NO_TM_SPEC          (-33)
#define SFU_NO_FMT_SPEC         (-34)
#define SFU_OVER_MAX            (-35)
#define SFU_INVALID_ARG         (-36)
#define SFU_BAD_DATA            (-37)
#define SFU_FILE_ERR            (-38)
#define SFU_RESPEC_ERR          (-39)
#define SFU_WRITE_ERROR         (-40)
#define SFU_PRINTER_ERROR       (-41)
#define SFU_TERM_EARLY          (-42)
#define SFU_FRAME_ERR           (-43)
#define SFU_NOPLACE		(-44)
#define SFU_BAD_SSP		(-45)
#define SFU_SPDSHT_FULL		(-46)
#endif

/* routines used internally in ssp.lib */
int ssp_check(sps_ptr);
void ssp_clear_refs(sps_ptr ssp);
int update(sps_ptr ssp);

#endif
