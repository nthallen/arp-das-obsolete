#ifndef ARPFIT_H_INCLUDED
#define ARPFIT_H_INCLUDED

#include <vector>

class af_function {
  public:

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

enum eval_type_t { Eval_Const, Eval_Input, Eval_Full };

class af_expr_instance;

// virtual base class for expressions
class af_expression {
  public:
    af_expression( char *op );
	virtual fit_t evaluate( eval_type_t etype, af_expr_instance& inst );

	char *op; // This may become more sophisticated
    
};

// Internal functions
class af_expr_func : public af_expression {
  public:
	inline af_expr_func( char *op ) : af_expression( op ) {}
	af_expr_func( char *op, af_expression &op1 );
	af_expr_func( char *op, af_expression &op1, af_expression &op2 );

	void new_operand( af_expression& operand );
	std::vector<af_expression *> operands; // These are the formal operands
	// Full list of inputs (set?)
    // List of implicit inputs // or just add them to the full list?
};

class af_expr_instance {
  public:
  //    af_expression
  //    type
  //    value
  //    list of func partials
  //    list of my partials
  //    list of partial calculations
  //    list of actual arguments
  //  
  //  Expr_Instance {
  //	Attributes:
  //	  Expression
  //	  Type
  //	  actual_inputs
  //	  actual_params
  //	  list of derivative calculations
  //	  list of actual arguments
  //  }
};


#endif
