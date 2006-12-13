#include <stdio.h>
#include <stdlib.h>
#include <file_utils.h>

main () {
int i, fpd;
    i = filecounter("tst","tst",&fpd);
    printf("there are %d files and %d files per directory\n",i,fpd);
}
