/* stmnt.c af_statement functions */
#include "arpfit.h"
#include <sstream>

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
