#ifndef ARPFIT_H_INCLUDED
#define ARPFIT_H_INCLUDED

#include <vector>
#include <iostream>
#include "err.h"

typedef double fitval_t;

class af_function {
  public:
    char *name;
  //  Function {
  //    Attributes:
  //      list of formal parameters with types
  //      list of implicit parameters with types
  //      list of internal definitions with types
  //      list of internal parameter definitions
  //      list of implicit params
  //      has_implicit_inputs
  //      Expression
  //    Methods:
  //      Evaluate( [Eval_Const|Eval_Input|Eval_Full] );
  //  }
};

enum eval_type_t { Eval_Const, Eval_Input, Eval_Independent, Eval_Full };

class af_expr_instance;

// virtual base class for expressions
class af_expression {
  public:
    inline af_expression( CoordPtr where = NoPosition, char *opin = "" ) {
	  op = opin;
	  def = where;
	}
	char *parsed() const;
	//virtual fitval_t evaluate( eval_type_t etype, af_expr_instance& inst );
	virtual void printOn(std::ostream& strm) const = 0;
	virtual char *strval() const;

	char *op; // This may become more sophisticated
	CoordPtr def; // Where this node was defined
};

typedef af_expression *af_expression_p;

inline std::ostream& operator << (std::ostream& strm, const af_expression *ex ) {
  ex->printOn( strm );
  return strm;
}

//---------------------------------------------------------------------
// af_expr_func - function invocation
//---------------------------------------------------------------------
class af_expr_func : public af_expression {
  public:
	inline af_expr_func( CoordPtr where, char *op ) : af_expression( where, op ) {}
	af_expr_func( CoordPtr where, char *op, af_expression *op1 );
	af_expr_func( CoordPtr where, char *op, af_expression *op1, af_expression *op2 );
	void printOn(std::ostream& strm) const;

	void new_operand( af_expression *operand );
	std::vector<af_expression *> operands; // These are the formal operands
	//fitval_t evaluate( eval_type_t etype, af_expr_instance& inst );

	// Full list of inputs (set?)
    // List of implicit inputs // or just add them to the full list?
};

typedef af_expr_func *af_expr_func_p;

//---------------------------------------------------------------------
// af_expr_const - A Constant
//---------------------------------------------------------------------
class af_expr_const : public af_expression {
  public:
	inline af_expr_const( CoordPtr where, fitval_t val ) :
	   af_expression( where, "CONST" ) { value = val; }
    fitval_t value;
	//fitval_t evaluate( eval_type_t etype, af_expr_instance& inst );
	void printOn(std::ostream& strm) const;
};

//---------------------------------------------------------------------
// af_expr_vector - a vector
//---------------------------------------------------------------------
class af_expr_vector : public af_expression {
  public:
	inline af_expr_vector( CoordPtr where ) :
	  af_expression( where, "VECTOR" ) {}
    inline void add_element( af_expression *expr ) {
	  elements.push_back(expr);
	}
	std::vector<af_expression *> elements;
	//fitval_t evaluate( eval_type_t etype, af_expr_instance& inst );
	void printOn(std::ostream& strm) const;
};

typedef af_expr_vector *af_expr_vector_p;

//---------------------------------------------------------------------
// af_expr_vector_triple - a type of vector
//---------------------------------------------------------------------
class af_expr_vector_triple : public af_expression {
  public:
	af_expr_vector_triple( CoordPtr where, af_expression *expr1in,
	  af_expression * expr2in, af_expression *expr3in );
	af_expression *expr1, *expr2, *expr3;
	void printOn(std::ostream& strm) const;
};

//---------------------------------------------------------------------
// af_expr_lvalue - A reference to a variable
//---------------------------------------------------------------------
class af_expr_lvalue : public af_expression {
  public:
	inline af_expr_lvalue( CoordPtr where, char *var )
	  : af_expression( where, var ) { index = 0; }
	inline af_expr_lvalue( CoordPtr where, char *var, af_expression *idx )
	  : af_expression( where, var ) { index = idx; }
	void printOn(std::ostream& strm) const;
	af_expression *index; // must be an INPUT expression (or constant)
	// int key; // Eli instance
};

//---------------------------------------------------------------------
// af_expr_string - A string expression
//---------------------------------------------------------------------
class af_expr_string : public af_expression {
  public:
	inline af_expr_string( CoordPtr where, char *str )
	  : af_expression( where, "STRING" )
	{ string = str; }
	char *string;
	char *strval();
	void printOn(std::ostream& strm) const;
};

struct partial_rule {
  int my_partial;
  int func_partial;
  fitval_t coefficient;
};

class af_expr_instance {
  public:
	af_expr_instance(af_expression *ex);
	af_expression *expr;
	eval_type_t type;
	fitval_t value;
	fitval_t coefficient;
	std::vector<fitval_t> func_partials;
	std::vector<fitval_t> my_partials;
	std::vector<partial_rule> rules;
	std::vector<af_expr_instance *> actual_args;
	std::vector<af_expr_instance *> my_params;
};

#define NEW(x) new x

#endif
