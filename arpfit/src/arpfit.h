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

// eval_type_t specifies what type of evaluation to do:
// Eval_Const: Fold constant expressions but don't evaluate any non-constants
//             Used once per invocation.
// Eval_Input: Evaluate node of type Var_Input and Var_Const
//             Used once per fit
// Eval_Param: Evaluate nodes of type Var_Param and below
//             Used on each iteration of the fit
// Eval_Independent: Evaluate everything
// Eval_Derivatives: Evaluate everything and calculate derivatives too
enum eval_type_t { Eval_Const, Eval_Input, Eval_Param,
					Eval_Independent, Eval_Derivatives };

// var_type_t specifies variable storage classes as well as expression
//   categories:
// Var_None: Not defined - could be anything
// Var_Const: Constant - remains the same throughout all fits
// Var_Input: May be updated each time input is read, i.e. once per fit
// Var_Param: May change on each iteration of the fit
// Var_Independent: An input vector for which calculations are performed
//             element-by-element. These nodes vary the fastest.
enum var_type_t { Var_None, Var_Const, Var_Input, Var_Param, Var_Independent };

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
// Variable Definition: Defined here before reference in af_expr_lvalue
//   All elements of an array are declared to be the same type. Each
//   element can be assigned only once. PARAMs will have modifiers
//   associated with them: Initialize, Constrain, Fix. Initialize
//   and Fix expressions can be required to be unique, but Constrain
//   need not be. The Fix conditions will be handled elsewhere.
//   Since all of these things can apply to elements of an array
//   independently, 
//---------------------------------------------------------------------
enum constraint_type_t { Constrain_GE, Constrain_LE };
class af_constraint {
  public:
	inline af_constraint( constraint_type_t op, af_expression *expr_in ) {
	  type = op;
	  expr = expr_in;
	}
	constraint_type_t type;
	af_expression *expr;
};

// af_lvalue refers to a single scalar value
// (or an input vector that is not otherwise indexed?)
class af_lvalue {
  public:
	inline af_lvalue( CoordPtr where, int sym_in, var_type_t type,
	                  int indexed_in, int index_in ) {
	  def = where;
	  sym = sym_in;
	  declared_type = type;
	  indexed = indexed_in;
	  index = index_in;
	  assigned = 0;
	  initialized = 0;
	  fixed = 0;
	}
	void assign( af_expression *expr );
	void initialize( af_expression *expr );
	void fix( af_expression *expr );
	inline void constrain( constraint_type_t op, af_expression *expr ) {
	  constraints.push_back(af_constraint(op, expr));
	}

	CoordPtr def;
	int sym, indexed, index;
	var_type_t declared_type;
    af_expression *assigned;
	af_expression *initialized;
	af_expression *fixed;
	std::vector<af_constraint> constraints;
};
class af_variable {
  public:
	af_variable( CoordPtr where, var_type_t type, int sym,
					int indexed_in = 0, int length = 1 );

	CoordPtr def;
	int sym;
	int indexed;
	int declared_length;
	std::vector<af_lvalue *> elements;
};
typedef af_variable *af_variable_p;

//---------------------------------------------------------------------
// af_expr_lvalue - A reference to a variable
//---------------------------------------------------------------------
class af_expr_lvalue : public af_expression {
  public:
	af_expr_lvalue( CoordPtr where, af_variable *var );
	af_expr_lvalue( CoordPtr where, af_variable *var, int idx );
	void printOn(std::ostream& strm) const;
	void assign(af_expression *expr);
	af_variable *variable;
	int indexed;
	int index;
	// DefTableKey key; // Eli instance
};
typedef af_expr_lvalue *af_expr_lvalue_p;

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

//---------------------------------------------------------------------
// af_decl_list
//---------------------------------------------------------------------
class af_decl_list {
  public:
    inline af_decl_list() {}
	inline void add_decl(af_expr_lvalue *decl) {
	  decls.push_back(decl);
	}
	std::vector<af_expr_lvalue *> decls;
};
typedef af_decl_list *af_decl_list_p;

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
