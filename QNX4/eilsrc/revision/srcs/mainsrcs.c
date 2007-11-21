/*
SRCS: simple revision control system.
Files: mainsrcs.c datasrcs.c filesrcs.c globsrcs.h inclsrcs.h datasrcs.h filesrcs.h
eil schomp 1990

mainsrcs.c: contains function main(). command parser. user interface.
*/

#include "datasrcs.h"
#include "inclsrcs.h"
#include "filesrcs.h"

#define PROMPT printf("rcs> ")
#define SHOW_COMMANDS puts("\n Register Copy Update Version Delete Project Write List Help Quit\n");

char *cmds[]={"register","copy","update","version","delete","project","write","list","help","quit"};
#define NUMCMDS (sizeof(cmds)/sizeof(char *))

/* globals */
char *progname={NULS},
projname[FMSIZE]={NULS},
str[LINE]={NULS},
name[FMSIZE]={NULS},
sysname[FMSIZE]={NULS},
inmemory[FMSIZE]={NULS};
extern unsigned short int getcrc(char *);
extern char *getlocation(char *, char *);
int command, asker=1, fix=0;
short int floppy=0, dirty=0, ok=0, qu=0;
char option;
long int _STACK = 11000;

void routine(void (*funptr1)(void), void (*funptr2)(void), void (*funptr3)(void)) {
    switch (option) {
        case NULS : (*funptr1)(); break;
        case 'F': if (projname[0]!=NULS) (*funptr3)(); else error(0,NOPROJECT,0); break;
        case 'S': (*funptr2)(); break;
    }
}

void usage(int command) {
    switch (command) {
        case QUIT:     puts(" q[uit]");
                       puts(" exit");
                       break;
        case REGISTIR: puts(" r[egister] [-s|-f] [args]");
                       puts(" registers a project, system[-s], or specific file(s)[-f]");
                       break;
		case COPY:     puts(" c[opy] [-s|-f] [args]");
				       puts(" copies files from database floppy and registers them");
				       puts(" projectwide, systemwide[-s] or specific file(s)[-f]");
				       break;
        case UPDATE:   puts(" u[pdate] [-s|-f] [args]");
                       puts(" updates a project, a system[-s], or specific files(s)[-f]");
                       break;
		case VERSION:  puts(" v[ersion] [-s][-f] [args]");
				       puts(" commits changes to files to database, projectwide, systemwide[-s],");
				       puts(" or specific file(s)[-f]");
				       break;
        case DELETE:   puts(" d[elete] [-s|-f] [args]");
                       puts(" deletes a project, files of a system[-s] or file(s)[-f] from database");
                       break;
        case PROJECT:  puts(" p[roject] [<rcs_file>[.rcs]]");
                       puts(" switch to a <rcs_file>, show current project");
                       break;
        case WRITER:   puts(" w[rite] [args]");
                       puts(" request write permission for <arg(s)>");
                       break;
		case LIST:     puts(" l[ist] [-s|-f] [args]");
				       puts(" list info from database for a project, system(s)[-s] or file(s)[-f]");
				       break;
        case HELP:     puts(" h[elp] [<command>]");
                       puts(" show commands or usage of command");
                       break;
        default:       break;
    }
}

void args(void) {
    char *comm;
    option=NULS; name[0]=NULS;
    while ((comm=strtok(NULL,SEPARATORS)) !=NULL)
    switch (*comm) {
        case '-': case '/': option=toupper(*(++comm)); break;
        default: strcpy(name,comm);
    }
}

int getcommand(char *str) {
char  *comm; int i=0,j;
if ((comm=strlwr(strtok(str,SEPARATORS)))!=NULL) {
   for (;i<NUMCMDS;i++) {
     j=stcpma(cmds[i],comm);
     if (j && *(comm+j)==NULS)
	        switch (*comm) {
			case 'Q': case 'q': return(QUIT);
			case 'R': case 'r': return(REGISTIR);
			case 'C': case 'c': return(COPY);
			case 'U': case 'u': return(UPDATE);
			case 'V': case 'v': return(VERSION);
			case 'L': case 'l': return(LIST);
			case 'D': case 'd': return(DELETE);
			case 'P': case 'p': return(PROJECT);
	        case 'W': case 'w': return(WRITER);
			case 'H': case 'h': return(HELP);
	        }
   }
   error(comm, BADCOMMAND, 0); return(99);
}
else { PROMPT; return(getcommand(gets(str))); }
}

void helper(void) {
    if (name[0]==NULS) {
       SHOW_COMMANDS;
       usage(HELP);
    }
    else
       usage(getcommand(name));
}

breakfunction(void) {
    puts(" please use the quit command to exit");
    puts(" continue? y/n"); fflush(stdout);
    if (getconfirm('N')) {
        if (dirty) {
           printf(" write the modified %s file? y/n\n",projname); fflush(stdout);
           if (getconfirm('Y')) writefile(projname);
        }
        exit(0);
    }
   else {
      PROMPT;
      return(0);
    }
}

main(int argc,char **argv) {
   char *p;
   int i;
   progname=(char *)malloc(FMSIZE);
   strcpy(progname,argv[0]);
   p=strchr(progname,'.'); if (p) *p=NULS;
#ifdef QNX
   if (!name_locate("dfs",My_nid,0))
      puts(" DFS: Dos File System not active");
   signal(SIGINT,breakfunction);
#else      
   if (onbreak(breakfunction)) puts(" can't set break trap");
#endif
   if (argc>1) {
       if (argv[1][0]=='?') {
	       	printf("%s [<rcs_file>[.rcs]]\nthe existing <rcs_file> is updated for all references to the current system\n",progname);
		exit(0);
	}
	for (i=1;i<argc;i++)
	if (!stricmp(argv[i],"+fix")) fix=1;    
    else {
	   	strcpy(name,argv[i]);
	    getproject();
	}
   }
  
   SHOW_COMMANDS;

   PROMPT;

   /* get token */
   while ((command=getcommand(gets(str)))!=QUIT) {
             args();
		     switch (command) {
		     case REGISTIR: routine(add_project,add_sys,add_file); break;
		     case COPY:     routine(cop_project,cop_sys,add_file); break;
             case UPDATE:   routine(up_project,up_sys,up_file); break;
             case VERSION:  routine(com_project,com_sys,com_file); break;
             case DELETE:   routine(del_project,del_sys,del_file); break;
		     case LIST:     routine(list_project,list_sys,list_file); break;
             case PROJECT:  getproject(); break;
             case WRITER:   request(); break;
             case HELP:     helper(); break;
             default:       SHOW_COMMANDS;
         }
    PROMPT;
    }
    if (dirty) writefile(inmemory);
    freememory();
    fcloseall();
    exit(0);
}
