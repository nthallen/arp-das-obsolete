#include "arpfit.h"
#include <sstream>
#include <string.h>

//------------------------------------------------------------------
// af_expression virtual base class
//------------------------------------------------------------------
char *af_expression::strval() const {
  message( ERROR, "Expected String Value", 0, def );
}

// Should be char const *, but being passed to C
char *af_expression::parsed() const {
  std::ostringstream ss;
  printOn(ss);
  return strdup(ss.str().c_str());
}

//------------------------------------------------------------------
// af_expr_func  function expression
//------------------------------------------------------------------
af_expr_func::af_expr_func( CoordPtr where, char *op, af_expression *op1 )
  : af_expression( where, op ) {
  operands.push_back( op1 );
}

af_expr_func::af_expr_func( CoordPtr where, char *op,
		af_expression *op1, af_expression *op2 )
    : af_expression( where, op ) {
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
  strm << "CONST(" << value << ")";
}


//------------------------------------------------------------------
// af_expr_vector
//------------------------------------------------------------------
void af_expr_vector::printOn(std::ostream& strm) const {
  std::vector<af_expression *>::const_iterator elt;
  int first = 1;
  strm << "VECTOR[";
  for ( elt = elements.begin(); elt != elements.end(); ++elt ) {
	if ( first ) first = 0;
	else strm << ", ";
	strm << *elt;
  }
  strm << "]";
}

//------------------------------------------------------------------
// af_expr_vector_triple
//------------------------------------------------------------------
af_expr_vector_triple::af_expr_vector_triple( CoordPtr where,
  af_expression *expr1in, af_expression * expr2in, af_expression *expr3in )
  : af_expression( where, "VECTOR3" )
{
  expr1 = expr1in;
  expr2 = expr2in;
  expr3 = expr3in;
  if ( expr1in == 0 || expr3in == 0 )
    message(DEADLY, "expr1 or expr3 is zero in vector triple", 0, where );
}
void af_expr_vector_triple::printOn(std::ostream& strm) const {
  strm << expr1;
  if ( expr2 != 0 ) strm << " : " << expr2;
  strm << " : " << expr3;
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

//------------------------------------------------------------------
// af_expr_string  string expression
//------------------------------------------------------------------
char *af_expr_string::strval() { return string; }
void af_expr_string::printOn(std::ostream& strm) const {
  strm << "\"" << string << "\"";
}
