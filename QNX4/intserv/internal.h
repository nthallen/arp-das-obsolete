/* Definitions of functions that are needed internally by the
   server
*/
#ifndef IS_INTERNAL_INCLUDED
#define IS_INTERNAL_INCLUDED

#ifndef INTSERV_H_INCLUDED
  /* we need intserv.h for IntSrv_reply. It also includes 
     <sys/types.h> */
  #error intserv.h must be included before internal.h
#endif

extern pid_t expint_proxy;
extern int expint_irq;
extern int intserv_quit;
void expint_init( void );
void expint_reset( void );
pid_t far expint_handler( void );
void service_expint( void );
void expint_attach( pid_t who, char *cardID, unsigned short address,
					  int region, pid_t proxy, IntSrv_reply *rep );
void expint_detach( pid_t who, char *cardID, IntSrv_reply *rep );

#endif
