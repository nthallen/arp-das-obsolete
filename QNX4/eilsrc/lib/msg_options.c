#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <msg.h>

extern char *opt_string;

/* initialises messages from command line options */
void msg_init_options(char *default_hdr, int argcc, char *argvv[]) {

/* getopt variables */
extern char *optarg;
extern int optind, opterr, optopt;

    int c;
    nid_t memo_node;
    int verbose, sounds, sys;
    char errfilename[FILENAME_MAX] = {'\0'};
    char oarg[FILENAME_MAX] = {'\0'};
    char hdr[40] = {'\0'};

    verbose = sounds = 1;
    memo_node = -1;
    msg_init(default_hdr,0,1,-1,0,1,1);
    if (default_hdr) strncpy(hdr,default_hdr,39);
    opterr = 0;
    optind = 0;

    do {
	c=getopt(argcc,argvv,opt_string);
	switch (c) {
	 	case 'o': strncat(oarg,optarg,FILENAME_MAX-1); break;
	 	case 'e': strncat(errfilename,optarg,FILENAME_MAX-1); break;
	 	case 'h': strncpy(hdr,optarg,39);  break;
	 	case 'c': memo_node = (nid_t)atoi(optarg); break;
	 	case 'v': verbose = 0; break;
		case 's': sounds = 0; break;
		case 'y': sys = 0; break;
		case '?': msg(MSG_EXIT_ABNORM,"Invalid option -%c",optopt);
		default : break;
	}
    }  while (c!=-1);
    msg_init(hdr,errfilename,verbose,memo_node,oarg,sounds,sys);
    optind = 0;
    opterr = 1;
}
