/* memory.h Functions defined by the memory management library.
 * $Log$
 * Revision 1.2  1992/08/20  20:59:25  nort
 * Cleaned up after porting to QNX4
 *
 * $Id$
   Distilled December 12, 1989
*/
#ifndef MEMLIB_H_INCLUDED
#define MEMLIB_H_INCLUDED
#include <stdio.h>

/* The basic routines */
void *new_memory(unsigned int size);
void *cnew_memory(unsigned int size);
void *free_memory(void *ptr);
FILE *snfopen(char *name, char *mode, char **bufp);
void snfclose(FILE *fp, char *bufp);

/* Hooks for providing sophisticated recovery schemes */
void memory_alloc(int (*alloc_func)(void));
int memory_satisfied(void);

/* User-definable functions (defaults are available,
   but they do next to nothing)
*/
extern void (*memory_salvage)(void);

#ifdef MEM_LOG
  void memstart(void);
  void memend(void);
  void memlog(int code, long int addr, int size);
#endif

#endif
