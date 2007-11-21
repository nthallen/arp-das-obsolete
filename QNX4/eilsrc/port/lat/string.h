#ifndef _STRINGH
#define _STRINGH 1

/**
*
* External definitions for string services
*
*/
#ifndef _SIZE_T
#define _SIZE_T

typedef unsigned int size_t;

#endif

extern int stcarg(const char *, const char *);
extern int stccpy(char *, const char *, int);
extern int stcgfe(char *, const char *);
extern int stcgfn(char *, const char *);
extern int stcgfp(char *, const char *);
extern int stcis(const char *,const char *);
extern int stcisn(const char *,const char *);
extern int stclen(const char *);
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
extern char *stoh(short,char *);
extern char *stpblk(const char *);
extern char *stpbrk(const char *, const char *);
extern char *stpchr(const char *, int);
extern char *stpchrn(const char *, int);
extern char *stpcpy(char *, const char *);
extern char *stpdate(char *, int, const char *);
extern char *stpichr(const char *, int);
extern char *stpichrn(const char *, int);
extern char *stpsym(const char *, char *, int);
extern char *stptime(char *, int, const char *);
extern char *stptok(const char *, char *, int, const char *);
extern char *stpu_d(char *, unsigned int, char, int);
extern int strbpl(char **, int, const char *);
extern char *strcat(char *, const char *);
extern char *strchr(const char *, int);
extern int strcmp(const char *, const char *);
extern char *strichr(const char *, int);
extern int stricmp(const char *, const char *);
#define strcmpi stricmp		/* For Microsoft compatibility */
extern char *strcpy(char *, const char *);
extern size_t strcspn(const char *, const char *);
extern char *strdup(const char *);
extern void strins(char *, const char *);
#undef strlen
extern size_t strlen(const char *);
extern char *strlwr(char *);
extern void strmfe(char *, const char *, const char *);
extern void strmfn(char *, const char *, const char *, const char *, const char *);
extern void strmfp(char *, const char *, const char *);
extern char *strncat(char *, const char *, size_t);
extern int strncmp(const char *, const char *, size_t);
extern char *strncpy(char *, const char *, size_t);
extern int strnicmp(const char *, const char *, size_t);
extern char *strnset(char *, int, int);
extern char *strpbrk(const char *, const char *);
extern char *strrchr(const char *, int);
extern char *strrev(char *);
extern char *strrichr(const char *, int);
extern char *strset(char *, int);
extern void strsfn(const char *, char *, char *, char *, char *);
extern size_t strspn(const char *, const char *);
extern void strsrt(char **, int);
extern char *strtok(char *, const char *);
extern char *strupr(char *);
extern int stscmp(char *, char *);
extern int stspfp(char *, int *);
int strcoll(const char *,const char *);
char *strerror(int);
char *strstr(const char *,const char *);
size_t strxfrm(char *,const char *,size_t);
/**
*
* External definitions for memory block services
*
**/
extern char *memccpy(char *, const char *, int, unsigned int);
#undef memcmp
#undef memcpy
#undef memset
extern void *memchr(const void *, int, size_t);
extern int memcmp(const void *, const void *, size_t);
extern void *memcpy(void *, const void *, size_t);
extern void *memmove(void *,const void *, size_t);
extern void *memset(void *, int, size_t);
extern void movmem(const char *, char *, unsigned int);
extern void repmem(char *, const char *, size_t, size_t);
extern void setmem(char *, unsigned int, int);
extern void swmem(char *, char *, unsigned int);


/**
*
* Definitions for built-in functions
*
**/
#define memcmp(a,b,c) __builtin_mempcm((c),(a),(b))
#define memcpy(a,b,c) __builtin_memypc((c),(b),(a))
#define memset(a,b,c) __builtin_memtes((c),(b),(a))
#define strlen(a) __builtin_strlen((a))
/**
*
* Prototypes for built-in function names
*
**/
extern void *__builtin_memypc( size_t, void *, void *);
extern int __builtin_mempcm(size_t, void *, void *);
extern void *__builtin_memtes( size_t, int, void *);
extern size_t __builtin_strlen(const char *);

#endif
