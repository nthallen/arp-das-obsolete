/*
    DBR Data Generator TCP/IP connection in.
    Written June 1993 by Eil for QNX 4.
*/

/* includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <globmsg.h>
#include <das.h>
#include <dbr.h>
#include <nortlib.h>
#include <eillib.h>

/* defines */
#define HDR "inin"
#define OPT_MINE

/* global variables */
char *opt_string=OPT_DG_INIT OPT_DG_DAC_IN OPT_MSG_INIT OPT_CC_INIT OPT_MINE;

main( int argc, char **argv )  {
/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

/* local variables */
int sock;
int fd;
int i;
struct stat st;
struct sockaddr_in server;

    /* initialise msg options from command line */
    msg_init_options(HDR,argc,argv);
    BEGIN_MSG;

    /* initialisations */

    /* process command line args */
    opterr = 0;
    optind = 0;
    do {
		i=getopt(argc,argv,opt_string);
		switch (i) {
		    case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		    default : break;
		}
    }  while (i!= -1);

    if ( (sock=socket(AF_INET, SOCK_STREAM, 0)) < 0)
		msg(MSG_EXIT_ABNORM,"Can't open stream socket");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = 0;
	i = sizeof(server);

	if (bind(sock, (struct sockaddr *)&server, i))
		msg(MSG_EXIT_ABNORM,"Can't bind name to socket");

	if (getsockname(sock, (struct sockaddr *)&server, &i))
		msg(MSG_EXIT_ABNORM,"Can't get socket name");

	msg(MSG,"Socket has port #%d",ntohs(server.sin_port));

	listen(sock,1);

	if ( (fd = accept(sock, 0, 0)) == -1)
		msg(MSG_EXIT_ABNORM,"Can't accept a connection");

	if (dup2(fd, STDIN_FILENO)==-1) msg(MSG_EXIT_ABNORM,"Can't dup to stdin");
	free(argv[0]);
	argv[0]=malloc(5);
	strcpy(argv[0],"fdin");
	if (execvp("fdin", argv)==-1) msg(MSG_EXIT_ABNORM,"Can't exec fdin");
}
