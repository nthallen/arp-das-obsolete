#ifndef ARPFIT_H_INCLUDED
#define ARPFIT_H_INCLUDED

#include <vector>
#include <iostream>
#include "err.h"

typedef double fitval_t;

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

// virtual base class for expressions
class af_expression {
  public:
    inline af_expression( CoordPtr where = NoPosition, char *opin = "" ) {
	  op = opin;
	  def = where;
	}
	char *parsed() const;
	virtual fitval_t evaluate( eval_type_t etype ) = 0;
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

// class af_expr_instance {
//   public:
// 	af_expr_instance(af_expression *ex);
// 	af_expression *expr;
// 	eval_type_t type;
// 	fitval_t value;
// 	fitval_t coefficient;
// 	std::vector<fitval_t> func_partials;
// 	std::vector<fitval_t> my_partials;
// 	std::vector<partial_rule> rules;
// 	std::vector<af_expr_instance *> actual_args;
// 	std::vector<af_expr_instance *> my_params;
// };

//---------------------------------------------------------------------
// af_expr_func - function invocation
//---------------------------------------------------------------------
class af_function;
class af_expr_func : public af_expression {
  public:
	inline af_expr_func( CoordPtr where, char *op ) : af_expression( where, op ) {}
	af_expr_func( CoordPtr where, af_function *func );
	af_expr_func( CoordPtr where, char *op, af_expression *op1 );
	af_expr_func( CoordPtr where, char *op, af_expression *op1, af_expression *op2 );
	void printOn(std::ostream& strm) const;

	void new_operand( af_expression *operand );
	af_function *function;
	std::vector<af_expression *> operands; // These are the formal operands
	//fitval_t evaluate( eval_type_t etype );

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
	//fitval_t evaluate( eval_type_t etype );
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
	//fitval_t evaluate( eval_type_t etype );
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
std::ostream& operator << (std::ostream& strm, const af_decl_list *dl );

//---------------------------------------------------------------------
// Statements:
//---------------------------------------------------------------------
class af_statement {
  public:
	inline af_statement( CoordPtr where ) { def = where; }
	CoordPtr def; // Where it was defined
	virtual void printOn(std::ostream& strm) const = 0;
	char *af_statement::parsed() const;
};
typedef af_statement *af_statement_p;

inline std::ostream& operator << (std::ostream& strm, const af_statement *ex ) {
  ex->printOn( strm );
  return strm;
}

class af_stmnt_assign : public af_statement {
  public:
	inline af_stmnt_assign( CoordPtr where, af_expr_lvalue *lv, af_expression *val )
		  : af_statement( where ) {
	  lvalue = lv;
	  value = val;
	}
	void printOn(std::ostream& strm) const;
	af_expr_lvalue *lvalue;
	af_expression *value;
};

class af_stmnt_arr_assign : public af_statement {
  public:
	inline af_stmnt_arr_assign( CoordPtr where, af_decl_list *decls, af_expr_func *func )
	                  : af_statement( where ) {
	  lvalues = decls;
	  function = func;
	}
	void printOn(std::ostream& strm) const;
	af_decl_list *lvalues;
	af_expr_func *function;
};

class af_stmnt_fit : public af_statement {
  public:
	inline af_stmnt_fit( CoordPtr where, af_expr_lvalue *lv, af_expression *expr )
	            : af_statement( where ) {
	  lvalue = lv;
	  fitexpr = expr;
	}
	void printOn(std::ostream& strm) const;
	af_expr_lvalue *lvalue;
	af_expression *fitexpr;
};

class af_fix {
  public:
	inline af_fix( CoordPtr where, af_expr_lvalue *lv,
						  af_expression *expr = 0 ) {
	  def = where;
	  lvalue = lv;
	  value = expr;
	}
	CoordPtr def;
	af_expr_lvalue *lvalue;
	af_expression *value;
};

// Used for both fix and float
class af_stmnt_fixflt : public af_statement {
  public:
	inline af_stmnt_fixflt( CoordPtr where,
					  af_expression *cond = 0 )
					  : af_statement(where) {
	  condition = cond;
	}
	inline void addfixflt( CoordPtr where, af_expr_lvalue *lv,
					  af_expression *expr = 0 ) {
	  list.push_back(new af_fix( where, lv, expr ) );
	}
	void printOn(std::ostream& strm) const;
	std::vector<af_fix*> list;
	af_expression *condition;
};
class af_stmnt_fix : public af_stmnt_fixflt {
  public:
	inline af_stmnt_fix( CoordPtr where, af_expression *cond = 0 )
	  : af_stmnt_fixflt( where, cond ) {}
	void printOn(std::ostream& strm) const;
};
class af_stmnt_float : public af_stmnt_fixflt {
  public:
	inline af_stmnt_float( CoordPtr where, af_expression *cond = 0 )
	  : af_stmnt_fixflt( where, cond ) {}
	void printOn(std::ostream& strm) const;
};
typedef af_stmnt_fixflt *af_stmnt_fixflt_p;

class af_stmnt_constraint : public af_statement {
  public:
	inline af_stmnt_constraint( CoordPtr where, af_expr_lvalue *lv,
		constraint_type_t op_in, af_expression *expr )
		: af_statement( where ) {
	  lvalue = lv;
	  op = op_in;
	  limit = expr;
	}
	void printOn(std::ostream& strm) const;
	af_expr_lvalue *lvalue;
	constraint_type_t op;
	af_expression *limit;
};

class af_stmnt_init : public af_statement {
  public:
    inline af_stmnt_init( CoordPtr where, af_expr_lvalue *lv,
	             af_expression *expr )
				 : af_statement( where ) {
	   lvalue = lv;
	   value = expr;
	}
	void printOn(std::ostream& strm) const;
	af_expr_lvalue *lvalue;
	af_expression *value;
};

//---------------------------------------------------------------------
// Function Definition
//    Includes name, prototype, statements and a return expression.
//    In the case of an internal function, the statements and return
//    expression are null.
//---------------------------------------------------------------------

class af_function {
  public:
	inline af_function( CoordPtr where, char *name_in, af_decl_list *decls,
						af_expression *expr ) {
	  def = where;
	  name = name_in;
	  formal = decls;
	  retval = expr;
	}
	inline void statement( af_statement *s ) { statements.push_back(s); }

	CoordPtr def;
    char *name;
	af_decl_list *formal;
	std::vector<af_statement *> statements;
	af_expression *retval;
  //  Function {
  //    Attributes:
  //      list of implicit parameters with types
  //      list of internal definitions with types?
  //      list of internal parameter definitions?
  //      list of implicit params?
  //      has_implicit_inputs
  //      Expression
  //    Methods:
  //      Evaluate( [Eval_Const|Eval_Input|Eval_Full] );
  //  }
};
typedef af_function *af_function_p;

class af_stmnt_loop : public af_statement {
  public:
    inline af_stmnt_loop( CoordPtr where, af_function *con )
			  : af_statement( where ) {
	  context = con;
	}
	void printOn(std::ostream& strm) const;
	af_function *context;
};

struct partial_rule {
  int my_partial;
  int func_partial;
  fitval_t coefficient;
};

#define NEW(x) new x
typedef char *C_STR;

#endif
