#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <usage.h>

void usage(int exit_code, char *cmd, int num,...) {
int fd;
char *m;
char **u;
va_list ap;
int i;

if (!exit_code) fd = STDOUT_FILENO;
else fd = STDERR_FILENO;
va_start(ap,num);
write(fd, cmd, strlen(cmd));
write(fd, " ", 1);
for (i=0;i<num;i++) {
    u = va_arg(ap, char **);
    m = *u;    
    while (m) {
	write(fd, m, strlen(m));
	write(fd, "\n", 1);
	m=*(++u);
    }
}
va_end(ap);
exit(exit_code);
}
