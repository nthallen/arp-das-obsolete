#ifndef ARPFIT_H_INCLUDED
#define ARPFIT_H_INCLUDED

#include <vector>
#include <iostream>

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
	inline af_expression() { op = ""; }
    inline af_expression( char *opin ) { op = opin; }
	//virtual fitval_t evaluate( eval_type_t etype, af_expr_instance& inst );
	virtual void printOn(std::ostream& strm) const = 0;

	char *op; // This may become more sophisticated
};

inline std::ostream& operator << (std::ostream& strm, const af_expression *ex ) {
  ex->printOn( strm );
  return strm;
}

// Internal functions
class af_expr_func : public af_expression {
  public:
	inline af_expr_func( char *op ) : af_expression( op ) {}
	af_expr_func( char *op, af_expression *op1 );
	af_expr_func( char *op, af_expression *op1, af_expression *op2 );
	void printOn(std::ostream& strm) const;

	void new_operand( af_expression *operand );
	std::vector<af_expression *> operands; // These are the formal operands
	//fitval_t evaluate( eval_type_t etype, af_expr_instance& inst );

	// Full list of inputs (set?)
    // List of implicit inputs // or just add them to the full list?
};

class af_expr_const : public af_expression {
  public:
	inline af_expr_const( char *op ) : af_expression( op ) {}
    fitval_t value;
	//fitval_t evaluate( eval_type_t etype, af_expr_instance& inst );
	void printOn(std::ostream& strm) const;
};

class af_expr_lvalue : public af_expression {
  public:
	inline af_expr_lvalue( char *var ) : af_expression( var ) {}
	inline af_expr_lvalue( char *var, af_expression *idx )
	  : af_expression(var) { index = idx; }
	void printOn(std::ostream& strm) const;
	af_expression *index; // must be an INPUT expression (or constant)
	// int key; // Eli instance
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

#endif
