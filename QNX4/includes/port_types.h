#ifndef _PORT_TYPES_H
#define _PORT_TYPES_H
/* when you want types to have the same number of bytes across platforms */
#ifdef __QNX__
#define BYTE1 char
#define UBYTE1 unsigned char
#define UBYTE1_MAX UCHAR_MAX
#define BYTE2 short
#define UBYTE2 unsigned short
#define BYTE4 long
#define UBYTE4 unsigned long
#else
#define BYTE1 char
#define UBYTE1 unsigned char
#define UBYTE1_MAX UCHAR_MAX
#define BYTE2 short
#define UBYTE2 unsigned short
#define BYTE4 long
#define UBYTE4 unsigned long
#endif

#endif
