/* USEFUL.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/* This software is the property of Bomem and should be considered and      */
/* treated as proprietary information.  Refer to the "Source Code License   */
/* Agreement"                                                               */
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 USEFUL.H 18-Aug-93,15:13:38,`THOMAS' File creation
1:1 USEFUL.H 14-Oct-93,11:32:02,`MICHEL' Added error base for IPSL
1:2 USEFUL.H 2-Dec-93,11:45:36,`THOMAS' Added new function to bigmem
     bigmemchr() that does the same as memchr in the standard library
     but for huge vectors. The prototype for bigmem.c are in useful.h
1:3 USEFUL.H 14-Dec-93,16:51:42,`JEAN' Added recurring exit code
     #define
1:4 USEFUL.H 17-Mar-94,12:05:22,`JEAN'
     Included the display_copyright() macro.
1:5 USEFUL.H 17-Mar-94,12:32:20,`JEAN' Added the str0cat() macro.
1:6 USEFUL.H 15-Apr-94,9:29:24,`JEAN' Added the new TLIB header
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

/* Various general purpose defines and function prototypes */

#ifndef	BOMEM_USEFUL
#define	BOMEM_USEFUL

#ifndef __STDIO_H
	#include <stdio.h> 				/* Required for FILE definition */
#endif
/*
	Undefine this symbol if the operating system support 32 addressing without
	segmentation
*/
#define SEGMENTED_OS

#ifdef SEGMENTED_OS
	#define HPTR		huge			/* Vector of more than 64kb */
										/* BIGMEM.C */
	/* To be replaced by C run time in 32bits implementation */
	void bigmemmove (void HPTR *dest, void HPTR *source, long n);
	void bigmemset (void HPTR *dest, int ch, long n);
	short bigmemcmp (void HPTR *block1, void HPTR *block2, long n);
	void HPTR *bigmemchr (void HPTR *dest, int ch, long n);
#else
	#define HPTR
	#define bigmemmove(dest, source, n)   memmove(dest, source, n)
	#define bigmemset (dest, ch, n)       memset (dest, ch, n)
	#define bigmemcmp (block1, block2, n) memcmp (block1, block2, n)
	#define bigmemchr (dest, ch, n)       memchr (dest, ch, n)
#endif

#define TRUE		1					/* DO NOT CHANGE THESE VALUES, "IF" */
#define FALSE		0					/* RELIES ON THESE VALUES */

#define USER_ABORT			1			/* Aborted by user */

#ifndef M_PI
	#define M_PI		3.14159265358979323846
#endif
#ifndef M_LN10
	#define M_LN10      2.30258509299404568402
#endif
#ifndef M_LN2
	#define M_LN2       0.693147180559945309417
#endif
#ifndef M_LOG10E
	#define M_LOG10E    0.434294481903251827651
#endif

#ifndef C87_STACK_SIZE
	#define C87_STACK_SIZE 		96	 	/* Memory size required for XXX87 */
#endif

#define FLASER			15799.7			/* Default sampling laser frequency */
#define FLASER_OLD		15798.012
#define FLASER_RAMAN	9394.5			/* Default RAMAN laser frequency */

extern double bo_flaser;				/* Actual laser frequency */

#ifdef __WATCOMC__		/* Force Microsoft calling sequence for asm functions */
	#pragma aux ASM_RTN  "_*" parm caller [] modify [ax bx cx dx];
#endif

/* Data type associated with equally spaced Y data (spectrum, ...) */
typedef struct
 	{
	float HPTR *buffer;
	long npts;
	double firstx;
	double lastx;
	}YDATA;

#define free_ydata(pt) bo_free ((pt).buffer)

typedef unsigned char byte;				/* Shortcut for unsigned char */
typedef unsigned short word;			/* Shortcut for unsigned short */

										/* SAVE87.ASM */
#ifdef __cplusplus
extern "C" {
#endif

void save_87 (char *stack_data);
void rstor_87 (char *stack_data);

#ifdef __cplusplus
	}
#endif

#ifdef __WATCOMC__
	#pragma aux (ASM_RTN) save_87;
	#pragma aux (ASM_RTN) rstor_87;
#endif
										/* BOALLOC.C */
void HPTR *bo_alloc (long length);
void HPTR *bo_realloc (void HPTR *block, long length);
void bo_free (void HPTR *ptr);
void *bo_salloc (unsigned short length);
char *bo_strdup(char *s);
void *bo_srealloc(void *block, unsigned short size);
void bo_sfree (void *ptr);
										/* BOALLOCD.C */
void HPTR *bo_allocd (long length, char *file, short line);
void HPTR *bo_reallocd (void HPTR *block, long length, char *file, short line);
void bo_freed (void HPTR *ptr, char *file, short line);
void *bo_sallocd (unsigned short length, char *file, short line);
char *bo_strdupd(char *s, char *file, short line);
void *bo_sreallocd(void *block, unsigned short size, char *file, short line);
void bo_sfreed (void *ptr, char *file, short line);
short bo_heap_checkd(char *file, short line);
void bo_dump_heapd(char *file, short line);
										/* GFORMAT.C */
short gformat (double x, short len, char *str);
short gformat0(double x, short len, byte fill, char *str);

										/* STR0CPY.c */
#ifndef	size_t
#include <string.h>
#endif
void str0cpy (char *dest, const char *src, size_t length);
#define str0cat( dest, src, length ) \
		str0cpy( dest+strlen(dest), src, length-strlen(dest))
										/* strdelb.c */
void strdelblks(char *s);

										/* BO_ERROR.C */
char *get_error_message(char *path, char *filename, short errnum);

										/* BOFCLOSE.C */
int bo_fclose(FILE *stream);

										/* BO_FILE.C */
FILE *bo_fopend(const char *path, const char *mode, char *sfile, short sline);
int bo_fclosed(FILE *stream, char *sfile, short sline);
void bo_dump_file_list(char *sfile, short sline);


#ifdef BO_DEBUG
	#define bo_alloc(len)			bo_allocd(len, __FILE__, __LINE__)
	#define bo_realloc(ptr, len)    bo_reallocd(ptr, len, __FILE__, __LINE__)
	#define bo_free(ptr)			bo_freed(ptr, __FILE__, __LINE__)
	#define bo_salloc(len)			bo_sallocd(len, __FILE__, __LINE__)
	#define bo_strdup(str)			bo_strdupd(str, __FILE__, __LINE__)
	#define bo_srealloc(ptr, len)   bo_sreallocd(ptr, len, __FILE__, __LINE__)
	#define bo_sfree(ptr)			bo_sfreed(ptr, __FILE__, __LINE__)
	#define bo_heap_check()			(void)bo_heap_checkd(__FILE__, __LINE__)
	#define bo_dump_heap()			bo_dump_heapd(__FILE__, __LINE__)
#else
	#define bo_heap_check()
	#define bo_heap_checkd()		NO_ERROR
	#define bo_dump_heap()
#endif

#ifdef FILE_DEBUG
	#define fopen(path, mode) 		bo_fopend(path, mode, __FILE__, __LINE__) 
	#define fclose(fptr)			bo_fclosed(fptr, __FILE__, __LINE__) 
	#define dump_file_list()		bo_dump_file_list(__FILE__, __LINE__) 
#else
	#define fclose(stream) 			bo_fclose(stream) 
	#define dump_file_list()
#endif




#ifdef __cplusplus
inline void *operator new(size_t size)
{
	return(bo_salloc((unsigned short)size));
}

inline void operator delete(void *block)
{
	bo_sfree(block);
}

/* Microsoft C++ does not yet support templates! */
#ifdef _MSC_VER
inline short    max(short a, short b)       {return((a > b) ? a : b);}
inline int      max(int a, int b)           {return((a > b) ? a : b);}
inline unsigned max(unsigned a, unsigned b) {return((a > b) ? a : b);}
inline long     max(long a, long b)         {return((a > b) ? a : b);}
inline float    max(float a, float b)       {return((a > b) ? a : b);}
inline double   max(double a, double b)     {return((a > b) ? a : b);}

inline short    min(short a, short b)       {return((a < b) ? a : b);}
inline int      min(int a, int b)           {return((a < b) ? a : b);}
inline unsigned min(unsigned a, unsigned b) {return((a < b) ? a : b);}
inline long     min(long a, long b)         {return((a < b) ? a : b);}
inline float    min(float a, float b)       {return((a < b) ? a : b);}
inline double   min(double a, double b)     {return((a < b) ? a : b);}
#endif

#endif

/* General error message numbers */
#define NO_ERROR				0
#define ERROR					(-1)
#define INVALID_FILE			(-2)
#define INVALID_DIRECTORY		(-3)
#define FILE_IO_ERROR			(-4)
#define FILE_NOT_FOUND			(-5)
#define NOT_ENOUGH_MEMORY		(-6)
#define TIMEOUT					(-7)
#define TOO_MANY_FILES			(-8)
#define UNINITIALIZED			(-9)
#define ERROR_STRUCT_ID			(-10)
#define ERROR_MISSING_PARM		(-11)
#define ERR_INVALID_ARGUMENT 	(-12)

/* Specific (contextual) error messages */
#define ERROR_BASE_ACQ		(-100)		/* -100 to -200 */
#define ERROR_BASE_QUANT	(-200)		/* -200 to -400 */
#define ERROR_BASE_SETUP	(-400)		/* -400 to -500 */
#define ERROR_BASE_GAL		(-500)		/* -500 to -600 */
#define ERROR_BASE_VECTOR	(-600)		/* -600 to -700 */
#define ERROR_BASE_TENSOR	(-700)
#define ERROR_BASE_IO_BOARD	(-800)
#define ERROR_BASE_DISP		(-900)

#define ERROR_BASE_OPTO		(-1000)		/* -1000 to -1100 */
#define ERROR_BASE_MATRIX	(-1100)		/* -1100 to -1200 */
#define ERROR_BASE_BTREE    (-1200)     /* -1200 to -1100 */

#define ERROR_BASE_DISCRIM	(-2000)	    /* -2000 to -2099 */
#define ERROR_BASE_XSERIAL	(-2100)		/* -2100 to -2199 */
#define ERROR_BASE_ECPC    	(-2200)		/* -2200 to -2299 */
#define ERROR_BASE_IPSL    	(-2300)		/* -2300 to -2399 */

#define ERROR_USER_BASE		(-20000)


/* Recurring exit code ( arbitrary ) */
#define EXIT_NORMAL		0
#define EXIT_USER_ABORT	1
#define EXIT_ERROR		23



/* Copyright macro */
/* To use this just define PROGRAM_NAME, PROGRAM_YEAR 	*/
/* PROGRAM_VERSION to the appropriate string value	*/
#define display_copyright() puts("\n Copyright (c) " PROGRAM_YEAR	\
								 " Bomem Inc.\n " PROGRAM_NAME 		\
								 " version " PROGRAM_VERSION "\n" )



#endif
