/* Col_main.c */
#include "Collector.h"

int main( int argc, char **argv ) {
  collector col;
  // Make sure tm_info is defined
  col->init();
  col->operate();
  return 0;
}
