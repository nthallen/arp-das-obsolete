/* snafuerr.h defines error codes placed in external variable snafu.
 * $Log$
 * Revision 1.1  1992/08/12  19:41:01  nort
 * Initial revision
 *
 * $Id$
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
