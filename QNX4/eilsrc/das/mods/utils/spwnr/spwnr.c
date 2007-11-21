/* header files */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <process.h>
#include <string.h>
#include <sys/kernel.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <globmsg.h>
#include <cmdctrl.h>
#include <reboot.h>
#include <break.h>

void my_signalfunction(int sig_num) {
exit(0);
}

void main(int argc, char **argv) {
char str[MAX_MSG_SIZE];
char *my_dir;
char *ptr;
char *vec[80];
pid_t p, f;
reply_type r;
sigset_t set;
int i;
int reboot_if_cant_spawn;

signal(SIGQUIT,my_signalfunction);
my_dir = getcwd(NULL,0);
reboot_if_cant_spawn = atoi(argv[1]);
sigemptyset(&set);
sigaddset(&set,SIGQUIT);
p=getppid();

while (1) {
    r = DAS_OK;
    f = Receive(p,str,MAX_MSG_SIZE);
    /* protect from QUIT signal */
    sigprocmask(SIG_BLOCK,&set,0);
    str[MAX_MSG_SIZE-1]='\0';
    if (f==p)
      switch(str[0]) {
	case TASK_RESTART:
	    *(strchr(str+sizeof(death_type),' '))='\0';
	    for (i=0,ptr=strtok(str+sizeof(death_type)+strlen(str+sizeof(death_type))+1," ");
		    i<79 && ptr;ptr=strtok(NULL," "),i++)
		vec[i] = ptr;
	    vec[i] = NULL;
	    if (i > 79) r = DAS_BUSY;
	    if (chdir(str+sizeof(death_type))==-1) r = DAS_BUSY;
	    /* Reply must come before spawn */
	    Reply(p,&r,sizeof(reply_type));
	    /* no message telling user that couldnt spawn */
	    if (r==DAS_OK) {
		if (spawnvp(P_NOWAIT,vec[0],vec)==-1 && reboot_if_cant_spawn)
		    REBOOTSYS;
	    }
	    break;
	case DAS_RESTART:
	    if (chdir(my_dir)==-1) r = DAS_BUSY;
	    Reply(p,&r,sizeof(reply_type));
	    if (r==DAS_OK)
		/* exits with status 0 if system was successful */
		exit(WEXITSTATUS(system(str+sizeof(death_type))));
	    break;
	default:
	    r=DAS_UNKN;
	    Reply(p,&r,sizeof(reply_type));	    
    }
    sigprocmask(SIG_UNBLOCK,&set,0);
}
}
