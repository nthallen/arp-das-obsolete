/* memlib.h Functions defined by the memory management library.
 * $Log$
   Reorganized again February 28, 1991
   Ported to QNX February 13, 1991
   Distilled December 12, 1989
*/
/* The basic routines */
void *new_memory(unsigned int size);
void *cnew_memory(unsigned int size);
void *free_memory(void *ptr);
FILE *snfopen(char *name, char *mode, char **bufp);
void snfclose(FILE *fp, char *bufp);

/* User-definable functions (defaults are available,
   but they do next to nothing) Routine should
   return only if some memory has been freed.
   If nothing can be done, termination with a
   message is appropriate.
*/
extern void (*memory_salvage)(void);

#ifdef MEM_LOG
  void memstart(void);
  void memend(void);
  void memlog(int code, long int addr, int size);
#endif
