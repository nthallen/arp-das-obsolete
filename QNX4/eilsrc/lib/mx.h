#ifndef _MY_MX_INCLUDED_
#define _MY_MX_INCLUDED_
#ifdef __QNX__
#include <sys/sendmx.h>
#endif

#ifndef __QNX__
struct _mxfer_entry {
  void *mxfer_off;	/* this will serve as pointer */
  int   mxfer_len;	/* this will serve as length */
};
#endif

#ifdef __QNX__
#define setmx(A,B,C) _setmx(A,B,C)
#else
#define setmx(A,B,C) { \
  A->mxfer_off = B; \
  A->mxfer_len = C; \
}
#endif

#define MX_SZ sizeof(struct _mxfer_entry)

#endif

