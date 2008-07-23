/* sb_internal.h defines functions internal to the subbus library.
   $Log$
*/
#ifndef _SUBBUS_H
  #error subbus.h should be included before sb_internal.h
#endif

#undef read_subbus
#undef read_ack
#undef write_ack
#undef set_cmdenbl
#undef read_novram
#undef write_novram
#undef read_switches
#undef set_failure
#undef read_failure
#undef read_rstcnt
#undef read_pwrcnt
#undef subbus_name

unsigned int far read_subbus(unsigned int);
int far write_ack(unsigned int, unsigned int);
int far read_ack(unsigned int, unsigned int far *data);
void far set_cmdenbl(int value);
unsigned char far read_novram(unsigned int address);
void far write_novram(unsigned int address, unsigned char val);
unsigned int far read_switches(void);
void far set_failure(int value);
unsigned char far read_rstcnt(void);
unsigned char far read_pwrcnt(void);
unsigned char far read_failure(void);
void cleanup(void);
void lenable_nmi(void);
void ldisable_nmi(void);
void lltick_sic(void);
void ldisarm_sic(void);
void lreboot(unsigned char critstat);
void init_subbus(void);
extern struct sbf sbfs;
extern char subbus_name[];
void nvram_init(void);
void nvram_dir(pid_t who, unsigned char id, unsigned short n_bytes);
