/* memory.h Functions defined by the memory management library.
 * $Log$
 * Revision 1.4  1994/06/20  18:42:56  nort
 * Added subset memlib functionality to nortlib
 * Made both includes cognizant of the fact
 *
 * Revision 1.3  1992/09/25  17:28:38  nort
 * Added conditional on MEMLIB_H_INCLUDED.
 * Included <stdio.h>
 *
 * Revision 1.2  1992/08/20  20:59:25  nort
 * Cleaned up after porting to QNX4
 *
 * $Id$
   Distilled December 12, 1989
*/
#ifndef MEMLIB_H_INCLUDED
#define MEMLIB_H_INCLUDED
#include <stdio.h>

#ifdef new_memory
  #error If memlib.h is required, it must preceed nortlib.h
#endif

/* The basic routines */
void *new_memory(size_t size);
void *cnew_memory(size_t size);
void *free_memory(void *ptr);
FILE *snfopen(char *name, char *mode, char **bufp);
void snfclose(FILE *fp, char *bufp);
char *nl_strdup(const char *s);
#define strdup(x) nl_strdup(x)

/* Hooks for providing sophisticated recovery schemes */
void memory_alloc(int (*alloc_func)(void));
int memory_satisfied(void);

/* Use of those hooks for limited memory allocation */
void *lnew_memory(size_t size);
void lfree_memory(void *ptr, size_t size);
extern long memory_available;

/* User-definable functions (defaults are available,
   but they do next to nothing)
*/
extern void (*memory_salvage)(void);

#ifdef MEM_LOG
  void memstart(void);
  void memend(void);
  void memlog(int code, long int addr, int size);
  void memory_check(void);
#else
  #define memory_check()
#endif

#if defined __386__
#  pragma library (memlib3r)
#elif defined __SMALL__
#  pragma library (memlibs)
#elif defined __COMPACT__
#  pragma library (memlibc)
#elif defined __MEDIUM__
#  pragma library (memlibm)
#elif defined __LARGE__
#  pragma library (memlibl)
# endif

#endif
