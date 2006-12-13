#ifndef ROLLOVER_H_INCL
#define ROLLOVER_H_INCL

/* rollover concerned comparisons */
#define LT(A,B,REF) ( (((B)>=(REF)) ? (A)<(B) : (A)>=(REF) || (A)<(B) ) )
#define EQ(A,B) ( (A)==(B) )
#define GTE(A,B,REF) ( !(LT(A,B,REF)) )
#define LTE(A,B,REF) ( LT(A,B,REF) || EQ(A,B) )
#endif
