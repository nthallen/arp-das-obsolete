// Definitions to compile acq_96.c with Watcom C++
#ifndef __cplusplus
  #error harvard.h is for use with C++
#endif
template<class T> T max(T a, T b) { return a>b?a:b; };
inline unsigned inportb(unsigned __port) { return inp(__port); }
inline unsigned inport(unsigned __port) { return inpw(__port); }
inline unsigned outportb(unsigned __port, unsigned __value) {
  return outp(__port, __value);
}
inline unsigned outport(unsigned __port, unsigned __value) {
  return outpw(__port, __value);
}
