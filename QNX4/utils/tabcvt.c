/* tabcvt.c Converts stdin tabs to stdout tabs
 * $Log$
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char rcsid[] = "$Id$";

#ifdef __USAGE
%C [-i n] [-o m]
	n specifies the input tab setting (default 8)
	m specifies the output tab setting (default 4)
	  A value of 0 means don't produce any tabs at all
	  in the output.
#endif

int nexttab(int c, int t) {
  if (t > 0) return((((c)/t)+1)*t);
  return(c);
}

int main(int argc, char **argv) {
  int intabs = 8, outtabs = 4, c, icol = 0, ocol = 0, nxtb;
  
  while (( c = getopt( argc, argv, "i:o:")) != -1) {
	switch (c) {
	  case 'i': intabs = atoi(optarg); break;
	  case 'o': outtabs = atoi(optarg); break;
	}
  }
  while ((c = getchar()) != EOF) {
	switch (c) {
	  case '\n': putchar(c); icol = ocol = 0; break;
	  case '\t': icol = nexttab(icol, intabs); break;
	  case ' ': icol++; break;
	  default:
		while (ocol < icol) {
		  nxtb = nexttab(ocol, outtabs);
		  if (icol - ocol == 1 || nxtb > icol) {
			putchar(' ');
			ocol++;
		  } else {
			putchar('\t');
			ocol = nxtb;
		  }
		}
		putchar(c);
		icol++;
		ocol++;
		break;
	}
  }
  return(0);
}
