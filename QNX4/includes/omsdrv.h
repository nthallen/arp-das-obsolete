/* omsdrv.h
   Header for Oregon Micro Systems Motor Controller
   Driver Library Interface
*/
#ifndef OMSDRV_H_INC
#define OMSDRV_H_INC

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned short USHRT;

/* Internal Definitions */

#define OMS_NAME "omsdriver"
#define OMS_SIG 'om'
#define OMS_CMD_MAX 80
typedef struct {
  USHRT signature;   /* OMS_SIG */
  USHRT function; /* omsmsgtype */
  USHRT n_req;
} Send_hdr_to_oms;
typedef struct {
  Send_hdr_to_oms hdr;
  char command[OMS_CMD_MAX];
} Send_to_oms;
/* n_req is required for read operations. It indicates how
   many separate responses are expected given the command
   text provided. e.g. "AXRPAYRP" will produce two
   responses. If a request is expected but not desired,
   a OMSMSG_READ_IGNORE is required with the appropriate
   n_req.
*/

typedef enum {
  OMSMSG_READ,
  OMSMSG_READ_LOG,
  OMSMSG_READ_IGNORE,
  OMSMSG_WRITE,
  OMSMSG_QUIT,
  OMSMSG_MAX
} omsmsgtype;

typedef struct {
  USHRT signature; /* 'om' */
  signed short status;
} Reply_hdr_from_oms;
typedef struct {
  Reply_hdr_from_oms hdr;
  char result[OMS_CMD_MAX];
} Reply_from_oms;
/* status: 0=success, 1=unknown command, -1=severe error */

int oms_init( void );
int oms_command( USHRT function, char *cmd, USHRT n_req, char *buf );
int oms_fprintf(char *fmt, ...);
int oms_fcommand( char axis, char *cmd, long distance, int go );

#define oms_shutdown() oms_command(OMSMSG_QUIT,NULL,0,NULL)
#define oms_read(c,n,b) oms_command(OMSMSG_READ,(c),(n),(b))
#define oms_write(c) oms_command(OMSMSG_WRITE,(c),0,NULL)
#define oms_drive(a,d) oms_fcommand((a),"MR",(d),1)
#define oms_drive_to(a,p) oms_fcommand((a),"MA",(p),1)
#define oms_set_speed(a,v) oms_fcommand((a),"VL",(v),0)
#define oms_preset(a,p) oms_fcommand((a),"LP",(p),0)

#ifdef __cplusplus
};
#endif

#if defined __386__
#  pragma library (nortlib3r)
#elif defined __SMALL__
#  pragma library (nortlibs)
#elif defined __COMPACT__
#  pragma library (nortlibc)
#elif defined __MEDIUM__
#  pragma library (nortlibm)
#elif defined __LARGE__
#  pragma library (nortlibl)
# endif

#endif
