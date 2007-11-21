#include <i86.h>

#define THIRD delay(166); nosound()
#define QUARTER delay(125); nosound()
#define HALF delay(250); nosound()
#define WHOLE delay(500); nosound()

#define FACTOR 1193180
#define OCTAVE 2

#define A ( FACTOR / (21694 / (1 << OCTAVE)) )
#define B ( FACTOR / (19328 / (1 << OCTAVE)) )
#define C ( FACTOR / (18242 / (1 << OCTAVE)) )
#define D ( FACTOR / (16252 / (1 << OCTAVE)) )
#define E ( FACTOR / (14479 / (1 << OCTAVE)) )
#define F ( FACTOR / (13666 / (1 << OCTAVE)) )
#define G ( FACTOR / (12175 / (1 << OCTAVE)) )
#define HI_A ( FACTOR / (21694 / (1 << (OCTAVE+1))))
#define HI_B ( FACTOR / (19328 / (1 << (OCTAVE+1))))
#define HI_C ( FACTOR / (18242 / (1 << (OCTAVE+1))))
#define HI_D ( FACTOR / (16252 / (1 << (OCTAVE+1))))
#define HI_G ( FACTOR / (12175 / (1 << (OCTAVE+1))))

#define FIRST_PART \
  sound(D); \
  THIRD;    \
  sound(D); \
  THIRD;    \
  sound(D); \
  THIRD;    \
  sound(G); \
  WHOLE;    \
  sound(HI_D); \
  WHOLE;

#define MIDDLE_PART \
  sound(HI_D); \
  THIRD; \
  sound(HI_C); \
  THIRD; \
  sound(HI_B); \
  THIRD; \
  sound(HI_G); \
  HALF; \
  sound(HI_D); \
  WHOLE; \
  HALF;


#define LAST_PART \
  sound(C); \
  QUARTER;  \
  sound(B); \
  QUARTER;  \
  sound(C); \
  QUARTER;  \
  sound(A); \
  WHOLE;


main() {
  FIRST_PART;
  MIDDLE_PART;
  MIDDLE_PART;
  LAST_PART;
}
