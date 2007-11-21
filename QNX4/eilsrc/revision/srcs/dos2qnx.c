/* define ASCII character values for linefeed, carriage return, control Z */
#include <stdio.h>
#include <stdlib.h>


#define  LF    10
#define  CR    13
#define  CTRLZ 26
#define  RS    30

enum state     { NONE, SAWCR, SAWLF, SAWCRLF, SAWCTRLZ };


int mstoix (FILE *infile, FILE *outfile)
{
   int c;
   enum state state = NONE;

   /* lone LF => unchanged, lone CR => unchanged,
      CR LF => LF, ^Z at end means EOF; ^Z elsewhere => unchanged */

   while (1) {       /* break out on EOF only */
      while ((c = getc (infile)) != EOF) {
         switch (c) {
          case LF:
            putc (c, outfile); if (state == SAWCR) state = NONE; break;
          case CR:
            state = SAWCR; break;
          case CTRLZ:
            if (state == SAWCR) putc (CR, outfile);
            state = SAWCTRLZ; goto saweof;
          default:
            if (state == SAWCR) { state = NONE; putc (CR, outfile); }
            putc (c, outfile);
            break;
         }
      }
 saweof:
      /* exit outer loop only on EOF or ^Z as last char */
      if (c = getc (infile) == EOF)
         break;
      else
         ungetc (c, infile);
      if (state == SAWCTRLZ)
         putc (CTRLZ, outfile);
   }
   return (0);
}


/* convert from ms-dos to qnx format */
int mstoqnx (FILE *infile, FILE *outfile)
{
   int c;
   enum state state = NONE;

   /* lone LF => unchanged, lone CR => unchanged,
      CR LF => RS, ^Z at end means EOF(early dos) remove!; ^Z elsewhere => unchanged */

   while (1) {       /* break out on EOF only */
      while ((c = getc (infile)) != EOF) {
         switch (c) {
          case LF:
            if (state == SAWCR) putc (RS, outfile);
            if (state == SAWCR) state = NONE; break;
          case CR:
            state = SAWCR; break;
          case CTRLZ:
            if (state == SAWCR) putc (CR, outfile);
            state = SAWCTRLZ; goto saweof;
          default:
            if (state == SAWCR) { state = NONE; putc (CR, outfile); }
            putc (c, outfile);
            break;
         }
      }
 saweof:
      /* exit outer loop only on EOF or ^Z as last char */
      if (state == SAWCTRLZ || (c = getc (infile)) == EOF)
         break;
      else
         ungetc (c, infile);
      if (state == SAWCTRLZ)
         putc (CTRLZ, outfile);
   }
   return (0);
}


/* convert from **ix to ms-dos format */
int ixtoms (FILE *infile, FILE *outfile)
{
   int c;
   enum state state = NONE;

   /* LF => CR LF, but leave CR LF alone */
   while ((c = getc (infile)) != EOF) {
      switch (c) {
       case LF:
         if (state == SAWCR)
            state = NONE;
         else
            putc (CR, outfile);
         putc (LF, outfile);
         break;
       case CR:
         state = SAWCR; putc (c, outfile); break;
       case CTRLZ:
         /* FALL THROUGH */
       default:
         state = NONE; putc (c, outfile); break;
      }
   }
   return (0);
}


main (argc, argv)
int argc;
char **argv;

{
   char option;
   FILE *outfile, *infile;
   int argn=1;
   
   if (argc < 2 || (*argv[argc]=='-') || (*argv[argc-1]=='-')) {
      puts(" flip [-d|-u|-q] infile outfile");
      exit(0);
    }
   infile=fopen(argv[argc-2],"r");
   outfile=fopen(argv[argc-1],"w");   

   option='q';      
   argopt (argc, argv, "", &argn, &option);
    switch (option) {
        case 'd': ixtoms(infile,outfile);
           break;
        case 'u': mstoix(infile,outfile);
           break;
        case 'q': mstoqnx(infile,outfile);
           break;
        }
        
  fclose(infile); fclose(outfile);      
}




