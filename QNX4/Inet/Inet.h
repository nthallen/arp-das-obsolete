#include <sys/types.h>

#define MYBUFSIZE 80
#define INET_REQUEST "Inetin v1.0"
int tmread( int socket, void *bfr, size_t nbytes );
void tmreadline( int socket, char *buf, int size );
int tmwrite( int socket, void *buf, int size );
void tmwritestr( int socket, char *str );
