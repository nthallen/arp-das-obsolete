/* FILEGEN.H */
/* ********************** COPYRIGHT (c) BOMEM INC, 1994 ******************* */
/* This software is the property of Bomem and should be considered and      */
/* treated as proprietary information.  Refer to the "Source Code License   */
/* Agreement"                                                               */
/* ************************************************************************ */

#if 0
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
1 FILEGEN.H 23-Jun-93,10:56:18,`THOMAS' File creation
1:1 FILEGEN.H 7-Oct-93,10:13:20,`MICHEL' Add prototypes for YDATA_RW
1:2 FILEGEN.H 17-Mar-94,13:40:30,`JEAN' This is test ( TLIB )
1:3 FILEGEN.H 11-Mar-94,18:37:42,`THOMAS'
1:4 FILEGEN.H 22-Mar-94,14:30:04,`THOMAS'
     Added support for filelist.c file listing module for batch processing.
1:5 FILEGEN.H 15-Apr-94,9:29:24,`JEAN' Added the new TLIB header
!!!!!! TLIB Revision history ( Do not remove ) !!!!!!
#endif

/* Various general purpose defines and function prototypes */
/* FOR FILE RELATED APPLICATIONS */

#ifndef BOMEM_FILEGEN
#define BOMEM_FILEGEN

#ifndef __STDIO_H
	#include <stdio.h>
#endif
#ifndef BOMEM_USEFUL
	#include "USEFUL.H"
#endif

/* prototype of functions in FLACCESS.C */
short flwrite (void HPTR *buffer, long length, FILE *file);
short flread  (void HPTR *buffer, long length, FILE *file);

/* prototype of functions in FILE_DIR.C */
char **get_dos_name_list(char *mask, short *err_code, long *n_element,
							  char sorted);
void free_dos_name_list(char **list);

/* prototype of functions in SETCWD.C */
short setcwd(char *path);

/* prototype of functions in FILECOMP.C */
short file_compare (char *s_name, char *d_name);

/* prototype of functions in FILECOPY.C */
short file_copy (char *s_name, char *d_name);

/* prototype of function in GETDIR.C */
short getdir (char drive, char *buffer);

/* prototype of function in YDATA_RW.C */
short ydata_disk_read (char file[], YDATA *spc);
short ydata_disk_write (char file[], YDATA *spc);

/* prototype of functions in FILELIST.C */
short filelist(char *name);
char *nextfile(short handle);
long numfiles(short handle);
void closelist(short handle);


#define MAX_DOS_DIR			(64+2)		/* Max path size including drive */
#define MAX_DOS_NAME		(8+1+3)
#define MAX_DOS_FULL_NAME	(MAX_DOS_DIR + MAX_DOS_NAME)

#endif
