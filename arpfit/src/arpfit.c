#include "arpfit.h"
#include <sstream>
#include <string.h>
#include "csm.h"

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
af_expr_func::af_expr_func( CoordPtr where, af_function *func )
  : af_expression( where, func->name ) {
  function = func;
}
af_expr_func::af_expr_func( CoordPtr where, char *op, af_expression *op1 )
  : af_expression( where, op ) {
  operands.push_back( op1 );
  function = 0;
}

af_expr_func::af_expr_func( CoordPtr where, char *op,
		af_expression *op1, af_expression *op2 )
    : af_expression( where, op ) {
  operands.push_back( op1 );
  operands.push_back( op2 );
  function = 0;
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
	break; // Only allow one
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
  if ( expr2 != 0 )
    strm << " : " << expr2;
  strm << " : " << expr3;
}

//------------------------------------------------------------------
// af_expr_lvalue  lvalue expression
//------------------------------------------------------------------
af_expr_lvalue::af_expr_lvalue( CoordPtr where, af_variable *var, int idx )
		: af_expression( where, "VAR" ) {
  index = idx;
  indexed = 1;
  variable = var;
  if ( variable != 0 && indexed ) {
	if ( ! variable->indexed ) {
	  message( ERROR, "Index on a non-indexed variable", 0, where );
	} else if ( index < 0 || index >= variable->declared_length ) {
	  message( ERROR, "Index out of range", 0, where );
	}
  }
}

af_expr_lvalue::af_expr_lvalue( CoordPtr where, af_variable *var )
				: af_expression( where, "VAR" ) {
  index = 0;
  indexed = 0;
  variable = var;
}

void af_expr_lvalue::printOn(std::ostream& strm) const {
  if ( variable == 0 ) strm << "[UndefinedVar]";
  else {
	strm << StringTable(variable->sym);
	if ( indexed ) {
	  strm << "[" << index << "]";
	}
  }
}

//------------------------------------------------------------------
// af_expr_string  string expression
//------------------------------------------------------------------
char *af_expr_string::strval() { return string; }
void af_expr_string::printOn(std::ostream& strm) const {
  strm << "\"" << string << "\"";
}


//------------------------------------------------------------------
// af_lvalue - a scalar value
//------------------------------------------------------------------
void af_expr_param::initialize( af_expression *expr ){
  if ( initialized != 0 ) {
	message( ERROR, "Variable initialized more than once", 0, expr->def );
	message( ERROR, "Previous initialization", 0, initialized->def );
  } else {
    if ( declared_type != Var_Param )
	  message( WARNING, "Cannot initialize a non-Param", 0, expr->def );
	initialized = expr;
  }
}

void af_expr_param::fix( af_expression *expr ){
  if ( fixed != 0 ) {
	message( ERROR, "Variable fix value defined more than once", 0, expr->def );
	message( ERROR, "Previous fix definition", 0, fixed->def );
  } else {
    if ( declared_type != Var_Param )
	  message( WARNING, "Cannot fix a non-Param", 0, expr->def );
	fixed = expr;
  }
}

//------------------------------------------------------------------
// af_variable - stored with the Key
//   length_in < 0 means the variable was not declared as a vector
//   legnth_in == 0 is illegal
//------------------------------------------------------------------
af_variable::af_variable( CoordPtr where, var_type_t type, int sym_in,
			  int indexed_in, int length_in ) {
  def = where;
  sym = sym_in;
  indexed = indexed_in;
  if ( length_in < 0 ) {
	message( DEADLY, "Invalid negative-length vector", 0, where );
  } else if ( length_in == 0 ) {
	message( ERROR, "Illegal zero-length vector", 0, where );
	length_in = 1;
  }
  if ( ! indexed && length_in != 1 ) {
    message( DEADLY, "Non-indexed variable cannot have length > 1", 0, where );
  }

  declared_length = length_in;
}

std::ostream& operator << (std::ostream& strm, const af_variable *ex ) {
  strm << StringTable(ex->sym);
  if ( ex->indexed ) {
    strm << "[" << ex->declared_length << "]";
  }
  return strm;
}
