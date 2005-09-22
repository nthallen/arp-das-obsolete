/* stmnt.c af_statement functions */
#include "arpfit.h"
#include <sstream>

/* -------------------------------------------------- */
/* Print functions                                    */
/* -------------------------------------------------- */
char *af_statement::parsed() const {
  std::ostringstream ss;
  printOn(ss);
  return strdup(ss.str().c_str());
}

void af_stmnt_assign::printOn(std::ostream& strm) const {
  strm << lvalue << " = " << value << ";" << std::endl;
}

void af_stmnt_arr_assign::printOn(std::ostream& strm) const {
  strm << "[" << lvalues << "] = " << function << ";" << std::endl;
}

void af_stmnt_fit::printOn(std::ostream& strm) const {
  strm << "Fit " << lvalue << " to " << fitexpr << ";" << std::endl;
}

void af_stmnt_fixflt::printOn(std::ostream& strm) const {
  std::vector<af_fix *>::const_iterator pos;
  int first = 1;
  for ( pos =  list.begin(); pos != list.end(); ++pos ) {
	if ( first ) first = 0;
	else strm << ", ";
	strm << (*pos)->lvalue;
	if ( (*pos)->value ) {
	  strm << " = " << (*pos)->value;
	}
  }
  if ( condition ) {
	strm << " if " << condition;
  }
  strm << ";" << std::endl;
}

void af_stmnt_fix::printOn(std::ostream& strm) const {
  strm << "Fix ";
  af_stmnt_fixflt::printOn(strm);
}

void af_stmnt_float::printOn(std::ostream& strm) const {
  strm << "Float ";
  af_stmnt_fixflt::printOn(strm);
}

void af_stmnt_constraint::printOn(std::ostream& strm) const {
  strm << "Constain " << lvalue;
  switch ( op ) {
	case Constrain_GE: strm << " >= "; break;
	case Constrain_LE: strm << " <= "; break;
	default: message(DEADLY,"Invalid Constrain_Op", 0, def); break;
  }
  strm << limit << ";" << std::endl;
}

void af_stmnt_init::printOn(std::ostream& strm) const {
  strm << "Initialize " << lvalue << " = " << value << ";" << std::endl;
}

void af_stmnt_loop::printOn(std::ostream& strm) const {
  strm << "while ()" << std::endl;
}

std::ostream& operator << (std::ostream& strm, const af_decl_list *dl ) {
  std::vector<af_variable *>::const_iterator pos;
  int first = 1;
  for ( pos =  dl->decls.begin(); pos != dl->decls.end(); ++pos ) {
	if ( first ) first = 0;
	else strm << ", ";
	strm << *pos;
  }
  return strm;
}

// Instantiate( instance, n_elts )
// makes sure the specified instance exists and that the specified
// number of elements is consistent with the definitions.
// The variable may have been declared as indexed with a specific
// declared_length, in which case, n_elts must either match or be
// zero. If the variable was not declared, then it's declared_length
// must be zero, and we'll use n_elts, even if it's zero.
void af_variable::Instantiate( const const int instance, int n_elts ) {
  int length = declared_length;
  if ( indexed ) {
	// Might need to allow n_elts == 1 in this case
	if ( n_elts != 0 && n_elts != declared_length ) {
	  message( ERROR, CatStrInd( "Declared length does not match assignment: ", sym ),
		0, NoPosition );
	}
  } else if ( declared_length != 0 ) {
	message( DEADLY, CatStrInd( "Expected zero length on unindexed var: ", sym ),
		0, NoPosition );
  } else {
	length = n_elts;
  }
  while ( instances.size() <= instance ) {
    instances.push_back(new af_var_inst( length ));
  }
  // check that the actual length is correct (may have been instantiated
  // somewhere else in the code?)
  int realsize = instances[instance]->elements.size();
  if ( realsize == 0 && length != 0 ) {
	instances[instance]->elements.resize(length,0);
  } else if ( realsize != 0 && length != 0 ) {
	message( DEADLY, CatStrInd( "Instance length mismatch: ", sym ));
  }
}

af_expression *af_variable::get_instance( const int instance, int index ) {
  if ( instances.size() <= instance ) {
	message( DEADLY, CatStrInd( "Attempt to get before instantiation: ", sym ),
	  0, NoPosition );
  }
  af_var_inst *vi = instances[instance];
  if ( vi->elements.size() <= index ) {
	message( DEADLY, CatStrInd( "Index out of range in get_instance: ", sym ),
	  0, NoPosition );
  }
  return vi->elements[index];
}

void af_variable::set_instance( const int instance, int index, af_expression *expr ) {
  if ( instances.size() <= instance ) {
	message( DEADLY, CatStrInd( "Attempt to set before instantiation: ", sym ),
	  0, NoPosition );
  }
  af_var_inst *vi = instances[instance];
  if ( vi->elements.size() <= index ) {
	message( DEADLY, CatStrInd( "Index out of range in set_instance: ", sym ),
	  0, NoPosition );
  }
  vi->elements[index] = expr;
}

//----------------------------------------------------------------
// Instantiation
// Check to make sure index is within range
// Check to make sure var has not already been instantiated
//   I will use zero length to indicate an as-yet-undefined length
//---------
// Perhaps I had better just disable assignment to declared vectors
// INDEPENDENTs will be considered scalars. In the future, assignment
// of a simple vector to an INDEPENDENT can be made legal, but for
// now, that can only be done via the array assignment syntax.
//----------------------------------------------------------------
void af_stmnt_assign::Instantiate( const int instance ) const {
  int lcl_indexed = indexed;
  int lcl_index = index;
  int lcl_length = lvalue->declared_length;
  if ( lvalue->indexed && ! indexed ) {
    message( DEADLY,
	  CatStrInd( "Invalid array assignment syntax: ", sym ),
	  0, def );
  }
  if ( indexed ) {
    if ( ! lvalue->indexed ) {
	  message( ERROR,
		CatStrInd( "Variable not declared as vector: ", lvalue->sym ),
		0, def );
	  lcl_indexed = 0; // so we can continue error checking
	} else if ( index >= lvalue->declared_length ) {
	  message( ERROR, "Index out of range", 0, def );
	  lcl_index = 0; // so we can continue error checking
	}
  } else {
    if ( index != 0 )
	  message( DEADLY, "index should be zero when not indexed", 0, def );
  }
  
  af_expr_instance *ei = value->Instantiate(instance);
  int rlen = ei->length();
  if ( rlen > 1 ) {
    message( DEADLY, "RHS is not a scalar", 0, def );
  }
  lvalue->Instantiate( instance, indexed ? lvalue->declared_length : 1 );
  if ( lvalue->get_instance( instance, lcl_index ) )
	message( DEADLY, CatStrInd( "Variable redefined: ", sym ), 0, def );
  lvalue->set_instance( instance, lcl_index, ei );
}

// LHS are all instantiated as INPUTs
// All args must be INPUTs
void af_stmnt_arr_assign::Instantiate( const int instance ) const {
}
void af_stmnt_fit::Instantiate( const int instance ) const {
}
void af_stmnt_fix::Instantiate( const int instance ) const {
}
void af_stmnt_float::Instantiate( const int instance ) const {
}
void af_stmnt_constraint::Instantiate( const int instance ) const {
}
void af_stmnt_init::Instantiate( const int instance ) const {
}
void af_stmnt_loop::Instantiate( const int instance ) const {
}
