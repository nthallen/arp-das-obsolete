#define SECDRIFT 90
#define TS_INTERVAL 10L
#define TS_MFC_LIMIT 32767

#define TSCHK_RTIME 1
#define TSCHK_IMPLICIT 2
#define TSCHK_CHECK 4
#define TSCHK_REQUIRED 8

#define D 0
#define A 1
#define U 2
#define DISCARD(x) status[x] = D
#define APPROVE(x) status[x] = A
#define UNAPPROVE(x) status[x] = U
#define DISCARDED(x) (status[x] == D)
#define APPROVED(x) (status[x] == A)
#define UNAPPROVED(x) (status[x] == U)
#define UNASSIGNED(x) ((x) < 0)
#define UNASSIGN(x) (x=-1)
