/* da_cache.h */
#ifndef DA_CACHE_H_INCLUDED
#define DA_CACHE_H_INCLUDED

typedef struct {
  unsigned short header;
  unsigned short type;
  unsigned short address;
  unsigned short value;
} cache_msg;

typedef struct {
  unsigned short header;
  unsigned short status;
  unsigned short value;
} cache_rep;

#define CACHE_MSG 'CA'
#define CACHE_READ 'rd'
#define CACHE_WRITE 'wr'
#define CACHE_QUIT 'qu'
  
#define CACHE_E_OK 0
#define CACHE_E_UNKN 1
#define CACHE_E_OOR 2
#define CACHE_E_NACK 3
#define CACHE_E_NOCACHE 4

#define CACHE_HW_MASK 0x7FFF
#define CACHE_NACK_MASK 0x8000

#define CACHE_NAME "dacache"

/* da_cache access */
int cache_write( unsigned short a, unsigned short v );
unsigned short cache_read( unsigned short a );
int cache_quit( void );

#if defined __386__
#  pragma library (nortlib3r)
#elif defined __SMALL__
#  pragma library (nortlibs)
#elif defined __COMPACT__
#  pragma library (nortlibc)
#elif defined __MEDIUM__
#  pragma library (nortlibm)
#elif defined __LARGE__
#  pragma library (nortlibl)
# endif

#endif
