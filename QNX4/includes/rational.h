/* rational.h contains definitions required for rational numbers.
   Written 1-26-87
   Moved to ps February 25, 1988
   Stolen for data use May 1, 1991

   $Id$
   $Log$
*/
#ifndef _RATIONAL_H
#define _RATIONAL_H

typedef struct {
  int num;
  int den;
} rational;

extern void rreduce(rational *);
extern void rplus(rational *, rational *, rational *);
extern void rminus(rational *, rational *, rational *);
extern void rtimes(rational *, rational *, rational *);
extern void rdivide(rational *, rational *, rational *);
extern void rtimesint(rational *a, int b, rational *c);
extern void rdivideint(rational *a, int b, rational *c);
int rcompare(rational *a, rational *b);
extern rational zero, one_half, one;
#endif
