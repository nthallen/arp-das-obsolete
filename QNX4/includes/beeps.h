/*
    beep sequences.
*/

#ifndef _BEEPS_H_INCLUDED
#define _BEEPS_H_INCLUDED

#include <i86.h>
#include <stdio.h>

#define PASS_BEEPS { \
putchar('\a'); \
fflush(stdout); \
}

#define WARN_BEEPS { \
putchar('\a');\
fflush(stdout); \
delay(350); \
putchar('\a'); \
fflush(stdout); \
}

#define FAIL_BEEPS { \
putchar('\a'); \
fflush(stdout); \
delay(350); \
putchar('\a'); \
fflush(stdout); \
delay(350); \
putchar('\a'); \
fflush(stdout); \
}

#endif
