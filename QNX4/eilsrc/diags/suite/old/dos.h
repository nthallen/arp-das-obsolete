#ifndef _DOS_H
#define _DOS_H
/**
*
* This header file supplies information needed to interface with the
* particular operating system and C compiler being used.
*
**/

/**
*
* The following type definition takes care of the particularly nasty
* machine dependency caused by the unspecified handling of sign extension
* in the C language.  When converting "char" to "int" some compilers
* will extend the sign, while others will not.  Both are correct, and
* the unsuspecting programmer is the loser.  For situations where it
* matters, the new type "byte" is equivalent to "unsigned char".
*
*/
#if LATTICE
typedef unsigned char byte;
#endif


/**
*
* Miscellaneous definitions
*
*/
#if FAMILY | DOS
#define SECSIZ 128		/* disk sector size */
#define DMA (char *)0x80	/* disk buffer address */
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif


/**
*
* The following symbols define the sizes of file names and node names,
* including the null terminators.
*/

#define FNSIZE 13	/* maximum file node size */
#define FMSIZE 64	/* maximum file name size */
#define FESIZE 4	/* maximum file extension size */

/**
*
* The following structures define the 8086 registers that are passed to
* various low-level operating system service functions.
*
*/
__noalign __nopad struct XREG
        {
        short ax,bx,cx,dx,si,di;
        };

__noalign __nopad struct HREG
        {
        unsigned char al,ah,bl,bh,cl,ch,dl,dh;
        };

union REGS
        {
        struct XREG x;
        struct HREG h;
        };

__noalign __nopad struct SREGS
        {
        short es,cs,ss,ds;
        };

__noalign __nopad struct XREGS
        {
        short ax,bx,cx,dx,si,di,ds,es;
        };

union REGSS
        {
        struct XREGS x;
        struct HREG h;
        };


/**
*
* The following codes are returned by the low-level operating system service
* calls.  They are usually placed into _OSERR by the OS interface functions.
*
*/
#define E_FUNC 1		/* invalid function code */
#define E_FNF 2			/* file not found */
#define E_PNF 3			/* path not found */
#define E_NMH 4	 		/* no more file handles */
#define E_ACC 5			/* access denied */
#define E_IFH 6			/* invalid file handle */
#define E_MCB 7			/* memory control block problem */
#define E_MEM 8			/* insufficient memory */
#define E_MBA 9			/* invalid memory block address */
#define E_ENV 10		/* invalid environment */
#define E_FMT 11		/* invalid format */
#define E_IAC 12		/* invalid access code */
#define E_DATA 13		/* invalid data */
#define E_DRV 15		/* invalid drive code */
#define E_RMV 16		/* remove denied */
#define E_DEV 17		/* invalid device */
#define E_NMF 18		/* no more files */

/**
*
* This structure contains disk size information returned by the getdfs
* function.
*/
__noalign __nopad struct DISKINFO
        {
        unsigned short free;	/* number of free clusters */
        unsigned short cpd;	/* clusters per drive */
        unsigned short spc;	/* sectors per cluster */
        unsigned short bps;	/* bytes per sector */
        };

/**
*
* The following structure is used by the dfind and dnext functions to
* hold file information.
*
*/
__noalign __nopad struct FILEINFO
        {
        char resv[21];			/* reserved */	
        char attr;			/* actual file attribute */
        long time;			/* file time  and date */
        long size;			/* file size in bytes */
        char name[FNSIZE];		/* file name */
        };


/**
*
* The following structure is a DOS device header.  It is copied to _OSCED
* when a critical error occurs.
*
*/
__noalign __nopad struct DEV
        {
        long nextdev;	/* long pointer to next device */
        short attr;	/* device attributes */
        short sfunc;	/* short pointer to strategy function */
        short ifunc;	/* short pointer to interrupt function */
        char name[8];	/* device name */
        };

/**
*
* The following structure contains country-dependent information returned
* by the getcdi function.
*
*/
__noalign __nopad struct CDI2		/* DOS Version 2 format */
        {
        short fdate;	/* date/time format */
                        /* 0 => USA (h:m:s m/d/y) */
                        /* 1 => Europe (h:m:s d/m/y) */
                        /* 2 => Japan (h:m:s d:m:y) */
        char curr[2];	/* currency symbol and null */
        char sthou[2];	/* thousands separator and null */
        char sdec[2];	/* decimal separator and null */
        char resv[24];	/* reserved */
        };

__noalign __nopad struct CDI3		/* DOS Version 3 format */
        {
        short fdate;	/* date format */
                        /* 0 => USA (m d y) */
                        /* 1 => Europe (d m y) */
                        /* 2 => Japan (d m y) */
        char curr[5];	/* currency symbol, null-terminated */
        char sthou[2];	/* thousands separator and null */
        char sdec[2];	/* decimal separator and null */
        char sdate[2];	/* date separator and null */
        char stime[2];	/* time separator and null */
        char fcurr;	/* currency format */
                        /* Bit 0 => 0 if symbol precedes value */
                        /*       => 1 if symbol follows value */
                        /* Bit 1 => number of spaces between value */
                        /*          and symbol */
			/* Bit 2 => 0 if bits 0-1 have meaning */
			/*       => 1 if curreny symbol replaces decimal */
        char dcurr;	/* number of decimals in currency */
        char ftime;	/* time format */
                        /* Bit 0 => 0 if 12-hour clock */
                        /*       => 1 if 24-hour clock */
        long pcase;	/* far pointer to case map function */
        char sdata[2];	/* data list separator and null */
        short resv[5];	/* reserved */
        };

__noalign __nopad struct CDI10		/* OS/2 format (See OS2NLS.H also) */
        {
        short cc;	/* country code */
        short cp;	/* default code page */
        short fdate;	/* date format */
                        /* 0 => USA (m d y) */
                        /* 1 => Europe (d m y) */
                        /* 2 => Japan (d m y) */
        char curr[5];	/* currency symbol, null-terminated */
        char sthou[2];	/* thousands separator and null */
        char sdec[2];	/* decimal separator and null */
        char sdate[2];	/* date separator and null */
        char stime[2];	/* time separator and null */
        char fcurr;	/* currency format */
                        /* Bit 0 => 0 if symbol precedes value */
                        /*       => 1 if symbol follows value */
                        /* Bit 1 => number of spaces between value */
                        /*          and symbol */
			/* Bit 2 => 0 if bits 0-1 have meaning */
			/*       => 1 if currency symbol replaces decimal */
        char dcurr;	/* number of decimals in currency */
        char ftime;	/* time format */
                        /* Bit 0 => 0 if 12-hour clock */
                        /*       => 1 if 24-hour clock */
        long resv2;	/* far pointer to case map function */
        char sdata[2];	/* data list separator and null */
        short resv3[5];	/* reserved */
        };

union CDI
        {
        struct CDI2 v2;
        struct CDI3 v3;
        struct CDI10 v10;
        };

/**
*
* Keyboard information structure and keyboard data structure for protected
* and family mode.  See OS2KBD.H for better definitions of the OS/2
* keyboard data structures and functions.
*
*/
__noalign __nopad struct KBDINFO
        {
        unsigned short size;	/* number of words in this structure	*/
        unsigned short mode;	/* mode flags, as follows		*/
                                /*	0 => echo on			*/
                                /*	1 => echo off			*/
                                /* 	2 => raw on			*/
                                /*	3 => raw off			*/
                                /* 	7 => set if 2-byte term char	*/
        unsigned short tchar;	/* line termination (turnaround) char */
        unsigned short iflag;	/* interim character flags */
        unsigned short shift;	/* shift flags */
        };

__noalign __nopad struct KBDDATA
        {
        unsigned char acode;	/* ASCII code */
        unsigned char scode;	/* scan code */
        unsigned char status;	/* status code */
        unsigned char nls_status; /* NLS shift status */
        unsigned short shift;	/* shift code */
        unsigned long time;	/* time stamp */
        };

/**
*
* Video structures for OS/2 and family mode.  See OS2VIO.H for better
* definitions.
*
*/
__noalign __nopad struct VCONFIG
        {
        short size;			/* structure size */
        short atype;			/* adaptor type */
        short dtype;			/* display type */
        long vram;			/* video RAM size */
        };

__noalign __nopad struct VMODE
        {
        short size;			/* size of structure */
        unsigned char type;		/* display type */
        unsigned char color;		/* color code */
        short ntc;			/* number of text columns */
        short ntr;			/* number of text rows */
        short npc;			/* number of pixel columns */
        short npr;			/* number of pixel rows */
        };


/**
*
* Level 0 I/O services
*
**/
extern char *cgets(char *);
extern int   cget(void);
extern int   cgetc(void);
extern void  chgdta(char __far *);
extern int   chgfa(const char *, int);
extern int   chgft(int, long);
extern int   dclose(int);
extern int   dcreat(const char *, int);
extern int   dcreatx(const char *, int);
extern int   ddup(int);
extern int   ddup2(int, int);
extern int   dfind(struct FILEINFO *, const char *, int);
extern int   dnext(struct FILEINFO *);
extern int   dopen(const char *, int);
extern unsigned dread(int, char *, unsigned);
extern long  dseek(int, long, int);
extern int   dunique(char *, int);
extern unsigned dwrite(int, const char *, unsigned);
extern int   getcd(int,char *);
extern int   getch(void);
extern int   getche(void);
extern int   getdfs(int, struct DISKINFO *);
extern char __far *getdta(void);
extern int   getfa(const char *);
extern int   getfc(int, int *);
extern long  getft(int);
extern int   getvfy(void);
extern int   isecho(void);
extern int   isadev(int);
extern int   iskbecho(void);
extern int   iskbhit(void);
extern int   iskbraw(void);
extern int   kbecho(int);
extern int   kbhit(void);
extern int   kbraw(int);
extern int   putch(int);
extern int   rlock(int, long, long);
extern void  rstdta(void);
extern void  rstvfy(void);
extern int   runlk(int, long, long);
extern int   settry(int, int);
extern void  setvfy(void);
extern int   ungetch(int);

/**
*
* Miscellaneous external definitions
*
*/
extern int bdos(int,...);
extern int bdosx(int,void *,...);
extern int chgclk(unsigned char *);
extern int chgdsk(int);
#undef cli
extern void cli(void);
extern int cputc(int);
extern int cputs(const char *);
extern int CXCERR(int);
extern int fndenv(char *);
#undef FP_OFF
extern unsigned FP_OFF(const void __far *);
#define OFF(fp)	(((unsigned *)&(fp))[0])
#undef FP_SEG
extern unsigned FP_SEG(const void __far *);
#define SEG(fp)	(((unsigned *)&(fp))[1])
extern long ftpack(const char *);
extern void ftunpk(long, char *);
extern int getbrk(void);
extern int getcdi(int, struct CDI3 *);
extern void getclk(unsigned char *);
extern void getcmd(char *);
extern int getdsk(void);
extern int getpf(char *, const char *);
extern int getpfe(char *, const char *);
#undef inp
extern unsigned char inp(unsigned);
extern int int86(int, union REGS *, union REGS *);
extern int int86s(int, union REGSS *, union REGSS *);
extern int int86x(int, union REGS *, union REGS *, struct SREGS *);
extern int intdos(union REGS *, union REGS *);
extern int intdoss(union REGSS *, union REGSS *);
extern int intdosx(union REGS *, union REGS *, struct SREGS *);
extern int isnet(void);
extern int isnetdc(int);
extern int isnetfh(int);
extern int isneton(void);
extern void __far *ltop(long);
extern void makedv(const char *, unsigned *, unsigned *);
extern char *makepp(const char *);
extern void makepv(int(*)(), unsigned *, unsigned *);
extern void movedata(unsigned, unsigned, unsigned, unsigned, unsigned);
extern int onbreak(int(*)());
extern void onerror(int);
#undef outp
extern void outp(unsigned, unsigned char);
extern void peek(unsigned, unsigned, char *, unsigned);
extern void poke(unsigned, unsigned, const char *, unsigned);
extern int poserr(const char *);
extern long ptol(const void __far *);
extern void rstbrk(void);
extern void rstdsk(void);
extern void segread(struct SREGS *);
extern int setcdi(int);
extern void setbrk(void);
#undef sti
extern void sti(void);
/**
*
* Memory allocation functions
*
*/
extern int getseg(unsigned, int, void __far **);
extern int rlsseg(void __far *);

/**
*
*	External Data Definitions
*
*/
extern char __far     *_CMD;	/* address of original command line */
/*	extern char	_DOS; or _DOS[2];	*/
extern char  __far    *_ENV;     /* address of original environment */
extern unsigned        _ENVL;    /* size of original environment (bytes) */
extern int   __private _FPERR;   /* floating point error code */
extern int             _MODEL;   /* model indicator */
extern char  __private _NDP;     /* non-zero if 8087 is installed */
extern short __private _NDPCW;   /* 8087 control word */
extern short __private _NDPSW;   /* 8087 status word */
extern char  _OSCEC;
extern struct DEV      _OSCED;
extern char  __private _OSCEF;
extern short _OSCET;
extern char  __private _OSERA;
extern char  __private _OSERC;
extern char  __private _OSERL;
extern short __private _OSERR;   /* DOS error code */
extern char  __private _PMODE;   /* processor mode, 0==real, 1==protect */
extern char __far     *_PRG;     /* address of fully qualified program name */
extern char __far     *_PSP;     /* address of PSP, valid if _XMODE==0 */
extern char __far * __private _SBASE;   /* address of stack section */
extern char  __private _SBIT;    /* segment increment bit number */
extern unsigned __private _SINC; /* segment increment value */
extern char            _SLASH;
extern unsigned        _SSIZE;   /* size of stack in bytes */
extern char	           _VER[];   /* compiler identification string */
extern char  __private _XMODE;   /* execution mode,*/
				/*	0==dos, */
				/*	1==family mode (dos or extended)*/
				/*	2==os2 1.1 */
/*
*	These macros assist in testing for the different execution
*	modes possible.
*/				
#define isdos()		(_XMODE == 0 || (_XMODE == 1 && !_PMODE))
#define isxdos()	(_XMODE == 1 && _PMODE)
#define isos2()		(_XMODE == 2)					

/*
*	Builtin definitions
*/
#define cli(x)	__builtin_cli(x)
#define FP_OFF(x)	__builtin_FP_OFF(x)
#define FP_SEG(x)	__builtin_FP_SEG(x)
#define inp(x)		__builtin_inp(x)
#define outp(x,y)		__builtin_outp(x,y)
#define sti(x)		__builtin_sti(x)

extern void __builtin_cli(void);
extern unsigned __builtin_FP_OFF(const void __far *);
extern unsigned __builtin_FP_SEG(const void __far *);
extern unsigned char __builtin_inp(unsigned);
extern void __builtin_outp(unsigned, unsigned char);
extern void __builtin_sti(void);

#endif


