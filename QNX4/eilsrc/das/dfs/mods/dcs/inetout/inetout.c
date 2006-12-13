/*
    Data client TCP/IP out.
*/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include "eillib.h"
#include "das.h"
#include "dbr.h"

/* defines */
#define HDR "inout"
#define OPT_MINE

/* global variables */
char *opt_string=OPT_DC_INIT OPT_MSG_INIT OPT_CC_INIT OPT_MINE;

main( int argc, char **argv) {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */	
int i, sock;
struct sockaddr_in server;
struct hostent *hp;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
		i=getopt(argc,argv,opt_string);
		switch (i) {
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default: break;
		}
    }  while (i!=-1);

    if (optind >= argc) msg(MSG_EXIT_ABNORM,"no host specified");
/*    if (argc > optind+1) msg(MSG_EXIT_ABNORM,"only one host allowed");*/

    if ( (sock=socket(AF_INET, SOCK_STREAM, 0)) < 0)
		msg(MSG_EXIT_ABNORM,"Can't open stream socket");

	server.sin_family = AF_INET;
	if ( (hp=gethostbyname(argv[optind])) == 0)
		msg(MSG_EXIT_ABNORM,"%s: unknown host",argv[optind]);

	memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	server.sin_port = htons(atoi(argv[optind+1]));

	if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
		msg(MSG_EXIT_ABNORM,"Can't connect");
		
	i=1;
	if (ioctl(sock, FIONBIO, &i) < 0)
		msg(MSG_EXIT_ABNORM,"Can't set non-block on socket");
		
	if (dup2(sock, STDOUT_FILENO)==-1)
		msg(MSG_EXIT_ABNORM,"Can't dup to stdout");
	free(argv[0]);
	argv[0]=malloc(6);
	strcpy(argv[0],"fdout");
	if (execvp("fdout", argv)==-1)
		msg(MSG_EXIT_ABNORM,"Can't exec fdout");
}
