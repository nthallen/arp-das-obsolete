/* header file for inclusion when using the library lat.lib */
/* lattice -> watcom (QNX 4) converter help library */

#include <limits.h>
#include <stdio.h>

#define FESIZE NAME_MAX
#define FNSIZE NAME_MAX
#define FMSIZE PATH_MAX

#define iscsym(c) ((isalnum((c))||((((c))&0xFF)==0x5F)))

/* simple defines, one to one correspondence between function */
#define clerr clearerr
#define stcis strspn
#define stcisn strcspn
#define stclen strlen
#define stpchr strchr
#define stpchrn strrchr
#define strcmpi stricmp


/* Lattice function definitions of functions in lat.lib */
extern FILE *fopene(const char *,const char *,char *);
extern int iabs(int);
extern int getfnl(const char *, char *, unsigned int, int);
extern char *argopt(int,const char**, const char *, int *, char *);
extern int stcarg(const char *, const char *);
extern int stccpy(char *, const char *, int);
extern int stcgfe(char *, const char *);
extern int stcgfn(char *, const char *);
extern int stcgfp(char *, const char *);
extern int stcd_i(const char *, int *);
extern int stcd_l(const char *, long *);
extern int stch_i(const char *, int *);
extern int stch_l(const char *, long *);
extern int stci_d(char *, int);
extern int stci_h(char *, int);
extern int stci_o(char *, int);
extern int stci_x(char *, int, int);
extern int stcl_d(char *, long);
extern int stcl_h(char *, long);
extern int stcl_o(char *, long);
extern int stcl_x(char *, long, int);
extern int stco_i(char *, int *);
extern int stco_l(char *, long *);
extern int stcpm(const char *, const char *, char **);
extern int stcpma(const char *, const char *);
extern int stcu_d(char *, unsigned int);
extern int stcul_d(char *, unsigned long);
extern char *stpblk(const char *);
extern char *stpbrk(const char *, const char *);
extern char *stpsym(const char *, char *, int);
extern char *stptok(const char *, char *, int, const char *);
extern char *stpu_d(char *, unsigned int, char, int);
extern int strbpl(char **, int, const char *);
extern void strins(char *, const char *);
extern void strmfe(char *, const char *, const char *);
extern void strmfn(char *, const char *, const char *, const char *, const char *);
extern void strmfp(char *, const char *, const char *);
extern void strsfn(const char *, char *, char *, char *, char *);
extern void strsrt(char **, int);
extern int stscmp(char *, char *);
extern int stspfp(char *, int *);



