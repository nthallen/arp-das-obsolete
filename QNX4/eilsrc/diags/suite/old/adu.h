/* adu.h defines functions from the ADU library. */
void arpx(int clrstk);
unsigned int getchl(void);
int  psettim(int control, int data, int ctrl_byte, int count);
void reload_vector(int intnum, int *new_vector, int *old_vector);
void store_dta(void);
int  sys_int(int ax, int dx);

/* ATEST.ASM    */
int atest(void);

/* CALL_DEV.ASM */
int call_dev(int dev, int cx, int dx);
int call_dev_nw(int dev, int cx, int dx);

/* CLR_PEV.ASM */
void clear_pev(void);

/* CURSOR.C     */
void sputch(int c);
void sys_cursor(int row, int col);

/* DISKPARK.C */
int diskpark(void);

/* DISP_MSG.C */
void set_msg(char *arg);
void disp_msg(int mode, char * cntrl, ...);

/* DISPMAX.C */
extern int msg_max;

/* DISPHDR.C */
extern char hdr[];

/* DISPMSG.C */
extern int msg;

/* ERROR.C */
int error(int err, char * cntrl,...);

/* EXIT_RR.C    */
void exit_rr(int opt);

/* INIT_DEV.C */
int init_device(int dev_num, void (far *att_rtn)(void),
     void (far *det_rtn)(), int q_length, int n_units);

/* ITOAG.C      */
char *itoag(int n, char *buf, int w, int base, int zeros, int signd);

/* ITOAG.C      */
char *litoag(long n, char *buf, int w, int base, int zeros, int signd);

/* MASK.C       */
void mask_ir(int mask);
void unmask_ir(int mask);
int is_cascaded(void);

/* PIC.C */
int picinit(void);
void sys_mask(int vector);
void sys_unmask(int vector);

/* PSETTIM.C            */
int psettim(int control, int data, int ctrl_byte, int count);

/* READ_DAT.C */
int read_date(char *buf);

/* REVERSE.ASM  */
unsigned char reverse(unsigned char);

/* SET_TIME.C */
int set_time(int mode, int count, int timer);

/* SEND_EOI.ASM */
void send_eoi(int vector);

/* SGETC.C      */
int agetch(void);
void aungetch(int c);

/* SIC_INIT.C */
void sic_init(char time_array[4], int new_log);

/* VECTORS.C    */
int set_vector(int vector_number, int *new_vector);
int reset_vector(int vector_number);
int reset_vectors(void);

/* GENERAL      */
void arpmain(int argc, char **argv);

