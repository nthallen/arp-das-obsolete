/* function.c */
#include "arpfit.h"

void af_function::Instantiate() {
  std::vector<af_statement *>::const_iterator s;
  int this_instance = instance_count++;
  
  // First instantiate all the statements
  for ( s = statements.begin(); s != statements.end(); s++ ) {
    (*s)->Instantiate( this_instance );
  }

  // Then instantiate the modifiers in pre-order
}

void af_function::Execute() {
  if ( ErrorCount[ERROR] != 0 || ErrorCount[DEADLY] != 0 ) {
    message( NOTE, "Skipping execution due to errors", 0, NoPosition );
    return;
  }
}
