/* Header file for the Topaz Program */

#define PAZ_PROXY_ID 6              /* used with nort's COL_set_proxy */
#define PAZ_SEND_STRING "Paz_data"  /* used with nort's Col_send_init */
#define PAZ "topaz"                 /* my attached name */

/* Structure that is passed to Collection */
/* These strings contain the same data that is sent by Topaz Power Supply
   except the terminating semicolon is replaced by NULL char '\000'. */
typedef struct {
  char d1_cur[7];          /* Diode 1 current */
  char d2_cur[7];          /* Diode 2 current */
  char d1_temp[7];         /* Diode 1 temperature */
  char d2_temp[7];         /* Diode 2 temperature */
  char d1_op_hrs[10];      /* Diode 1 operating hours */
  char d2_op_hrs[10];      /* Diode 2 operating hours */
  char d1_cur_setpt[5];    /* Diode 1 current setpoint */
  char d2_cur_setpt[5];    /* Diode 2 current setpoint */
  char d1_temp_setpt[4];   /* Diode 1 temperature setpoint */
  char d2_temp_setpt[4];   /* Diode 2 temperature setpoint */
  char d4_temp_setpt[5];   /* Doubler temperature setpoint */
  char rep_rate_setpt[6];  /* Rep Rate setpoint */
  char diode_event[2];     /* Diodes On/Off Event Queued */
  char paz_status[17];     /* History Status Array */
} Paz_frame;

unsigned short Convert_Topaz_Data(const char *s, int decimals);
