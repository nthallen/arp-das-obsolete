#include <sys/types.h>

#define MYBUFSIZE 80
int tmread( int socket, void *bfr, size_t nbytes );
void tmreadline( int socket, char *buf, int size );
void tmwrite( int socket, void *buf, int size );
