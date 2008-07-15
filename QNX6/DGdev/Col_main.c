/* Col_main.c */
#include "Collector.h"

int main( int argc, char **argv ) {
  // oui_init_options(argc, argv);
  collector col;
  col.init();
  col.operate();
  return 0;
}
