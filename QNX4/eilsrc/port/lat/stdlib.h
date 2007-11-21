#ifndef _STDLIBH
#define _STDLIBH 1

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 32767

#ifndef HUGE_VAL
extern const char _huge[];
#define HUGE_VAL (*(double *)_huge)
#endif

#ifndef _SIZE_T
#define _SIZE_T

typedef unsigned int size_t;

#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef unsigned int wchar_t;
#endif

#define MB_CUR_MAX _mb_cur_max
extern	int	__private _mb_cur_max;

typedef __noalign __nopad struct {
     int quot;
     int rem;
     } div_t;

typedef __noalign __nopad struct {
     long int quot;
     long int rem;
     } ldiv_t;

#if SFLAG & LPTR
#define p_big __huge
#else
#define p_big
#endif


/**
*
* ANSI memory allocation services
*
**/
extern void *malloc(size_t);
extern void *calloc(size_t,size_t);
extern void *realloc(void *,size_t);
#ifndef free
extern void free(void *);
#endif
extern int _free(void *);



/**
*
* Lattice memory allocation services
*
**/
extern int allmem(void);
extern void *getmem(unsigned int);
extern void p_big *getml(unsigned long int);
extern char __huge *halloc(unsigned long,unsigned);
extern int hfree(char __huge *);
extern int rlsmem(void *, unsigned int);
extern int rlsml(void p_big *, unsigned long int);
extern int bldmem(int);
extern long int sizmem(void);
extern long int chkml(void);
extern void rstmem(void);


/**
*
* UNIX memory allocation services
*
**/
extern char *sbrk( int );
extern char *lsbrk( long );
extern void rbrk(void);

/**
*
* Sort functions
*
*/
extern void dqsort(double p_big *, size_t);
extern void fqsort(float p_big *, size_t);
extern void lqsort(long p_big *, size_t);
extern void qsort(void p_big *, size_t, size_t, int (* )(const void *,const void *));
extern void sqsort(short p_big *, size_t);
extern void tqsort(char * p_big *, size_t);
extern void p_big *bsearch(const void *,const void p_big *,size_t,size_t,int (* )(const void *,const void p_big *));



/**
*
* fork/exec functions
*
*/
extern int execl(const char *, const char *,...);
extern int execv(const char *, const char **);
extern int execle(const char *, const char *,...);
extern int execve(const char *, const char **, const char **);
extern int execlp(const char *, const char *,...);
extern int execvp(const char *, const char **);
extern int execlpe(const char *, const char *,...);
extern int execvpe(const char *, const char **, const char **);

extern int forkl(const char *, const char *,...);
extern int forkv(const char *, const char **);
extern int forkle(const char *, const char *,...);
extern int forkve(const char *, const char **, const char **);
extern int forklp(const char *, const char *,...);
extern int forkvp(const char *, const char **);
extern int forklpe(const char *, const char *,...);
extern int forkvpe(const char *, const char **, const char **);

extern int wait(void);
extern int system(const char *);

/**
*
* Localization functions
*
**/
extern int mblen(const char *,size_t);
extern size_t mbstowcs(wchar_t *, const char *, size_t);
extern int mbtowc(wchar_t *, const char *, size_t);
extern size_t wcstombs(char *, const wchar_t *, size_t);
extern int wctomb(char *, wchar_t);


/**
*
* Miscellaneous functions
*
*/
extern void abort(void);
#undef abs
extern int abs(int);
extern int __builtin_abs(int);
extern char *argopt(int,const char**, const char *, int *, char *);
extern int atoi(const char *);
extern double atof(const char *);
extern long int atol(const char *);
extern void exit(int);
extern void _exit(int);
extern int fndenv(char *);
extern char *getenv(const char *);
extern char *getfirst(const char *, unsigned, int);
extern int getfnl(const char *, char *, unsigned int, int);
extern char *getnext(void);
extern int getpid(void);
extern int iabs(int);
extern int isauto(const void *);
extern int isdata(const void *, unsigned int);
extern int isdptr(const void *);
extern int isheap(const void *);
extern int ispptr(void(*)());
extern int isprivate(const void *);
extern int isstatic(const void *);
extern long int labs(long int);
extern int onexit(int(*)());
extern int putenv(const char *);
extern int rand(void);
extern int rmvenv(const char *);
extern void srand(unsigned int);
extern double strtod(const char *,const char **);
extern long int strtol(const char *, char **, int);
extern unsigned long int strtoul(const char *,char **,int);
extern long int utpack(const char *);
extern void utunpk(long int, char *);
extern int atexit(void (* )(void));
extern int atexit();
extern div_t div(int, int);
extern ldiv_t ldiv(long int, long int);
extern char *strerror(int);


#define abs(i) __builtin_abs((i))

/*------defines and variables for atexit function--------*/

__noalign __nopad struct _exit {
    void (*_ex_fptr)();
    __noalign __nopad struct _exit *_ex_next;
    };

extern __noalign __nopad struct _exit *_atexit;

#endif
