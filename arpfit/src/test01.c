#include "arpfit.h"

int main( int argc, char **argv ) {
  std::cout << "arpfit\n";
  af_expression *cexp3 = new af_expr_const("3");
  af_expression *cexp5 = new af_expr_const("5");
  af_expression *fexp = new af_expr_func("MUL",cexp3,cexp5);
  af_expression *lv = new af_expr_lvalue("var");
  af_expression *f2 = new af_expr_func("ADD", fexp, lv );
  std::cout << f2 << "\n";
  return 0;
}
