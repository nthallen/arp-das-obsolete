#include "arpfit.h"

//------------------------------------------------------------------
// af_expression virtual base class
//------------------------------------------------------------------

//------------------------------------------------------------------
// af_expr_func  function expression
//------------------------------------------------------------------
af_expr_func::af_expr_func( char *op, af_expression *op1 )
  : af_expression(op) {
  operands.push_back( op1 );
}

af_expr_func::af_expr_func( char *op, af_expression *op1, af_expression *op2 )
  : af_expression(op) {
  operands.push_back( op1 );
  operands.push_back( op2 );
}

void af_expr_func::printOn(std::ostream& strm) const {
  std::vector<af_expression *>::const_iterator pos;
  int first = 1;
  strm << op << "( ";
  for ( pos =  operands.begin(); pos != operands.end(); ++pos ) {
	if ( first ) first = 0;
	else strm << ", ";
	strm << *pos;
  }
  strm << " )";
}

void af_expr_func::new_operand( af_expression *operand ) {
  operands.push_back( operand );
}

//fitval_t af_expr_func::evaluate( eval_type_t etype, af_expr_instance& inst ) {
  // Define
//}


//------------------------------------------------------------------
// af_expr_const  constant expression
//------------------------------------------------------------------
void af_expr_const::printOn(std::ostream& strm) const {
  strm << op;
}

//------------------------------------------------------------------
// af_expr_lvalue  lvalue expression
//------------------------------------------------------------------
void af_expr_lvalue::printOn(std::ostream& strm) const {
  strm << op;
  if ( index ) {
	strm << "[" << index << "]";
  }
}
