#include <string.h>
#include "tabs.h"

char *tabs_to_space(char *txt) {
char *p;
char *t;
    if (!txt) return(NULL);
    t=txt;
    while ( (p=strchr(t,TAB))!=NULL) {
	*p=SPACE;
	t=p+1;
    }
    return(txt);
}
