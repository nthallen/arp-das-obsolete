#include <string.h>
#include <vector.h>

/* returns 0 on success */
int vector(char *str, char **v, int size) {
int i;
char *ptr;

for (i=0, ptr=strtok(str," "); i<(size-1) && ptr; ptr = strtok(NULL," "), i++)
    v[i]=ptr;
v[i]=NULL;
if (i>(size-1)) return(-1);
return(0);
}
