#include "arpfit.h"

af_expr_func::af_expr_func( char *op, af_expression &op1 )
  : af_expression(op) {
  operands.push_back( op1 );
}

af_expr_func::af_expr_func( char *op, af_expression &op1, af_expression &op2 ) {
  : af_expression(op) {
  operands.push_back( op1 );
  operands.push_back( op2 );
}
