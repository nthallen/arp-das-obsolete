//
// CString class
//
#include "CString.h"
#include <assert.h>
#include <string.h>
#include <iostream.h>
//********************************************************************
//*  Constructors, deconstructor, copy constructor                  **
//********************************************************************
CString::CString():szS(0),pS(0){}	// default constructor

CString::CString(const int n) {     // Allocate string of a specified size
	pS = new char[n + 1];            // allocate space plus a null
	assert(pS!=0);							// check allocation
	szS = n;									// save length
	*pS = '\0';								// set string to null
}

CString::CString(const char *psX) { // Convert a char * to a string
	szS = strlen (psX);				 // get length of parm
	pS = new char[szS + 1];			 // allocate
	assert( pS!=0 );					 // memory allocated OK?
	strcpy(pS, psX);					 // perform copy
}

CString::CString(const CString& str): // copy constructor
	szS(str.szS) {
		pS = new char[szS + 1];         // allocate space for string
		assert( pS!=0 );					  // check allocation
		strcpy(pS, str.pS );				  // perform copy
}

inline CString::~CString()			// deconstructor
		{ if (pS) delete[] pS; }

//********************************************************************
//*  Operators: Assignment, Concat, Comparison 		                 **
//********************************************************************

//*****************   Assignment *************************************
CString& CString::operator=(const CString& str){ // assignment
	if ( this == &str ) 				// copy to the same object?
		return *this;					// yes, skip
	if ( szS != str.szS) {			// different string lengths?
		if (pS) delete[] pS;       // delete old string
		szS = str.szS;             // copy size
		pS = new char[szS + 1];    // get new string area
	}
	assert (pS !=0);              // check
	strcpy(pS, str.pS);           // perform copy
	return *this;                 // return ref to string for mult assign
}

//*****************   Concatenate *************************************
CString& CString::operator+=( const CString &s) {
	szS += s.szS;						// calculate new length
	char *pTemp = new char[szS+1];// get new area
	assert (pS != 0);
	strcpy(pTemp, pS);            // copy base string
	strcat(pTemp, s.pS);          // copy concat string
	delete[] pS;                  // delete base string
	pS = pTemp;                   // point to new area
	return *this;                 // return for mult assign
}

//*****************  Concat char *  (for efficency) ******************
CString& CString::operator+=( const char * psParm) {
	szS += strlen(psParm);
	char *pTemp = new char[szS+1];
	assert (pS != 0);
	strcpy(pTemp, pS);
	strcat(pTemp, psParm);
	delete[] pS;
	pS = pTemp;
	return *this;
}
//***************** Concat Op + (friend)  ****************************
CString operator+ (const CString& s1, const CString& s2) {
	CString temp = s1;						// Make a copy to return
	temp += s2;									// use existing +=
	return temp;
}

//**************** Concat Op * (not friend) -- example ***************
CString CString::operator* (const CString &s) const {
	CString temp = *this;			// Make a copy to return
	temp += s;							// use existing +=
	return temp;
}


// -----------  logical operators --------------

int CString::operator==(const CString& s) const {
	 for( int i=0; pS[i] && s.pS[i] && pS[i]==s.pS[i]; i++);
	 return (pS[i]==s.pS[i]);
}

int CString::operator<(const CString& s) const {
	 return strcmp(pS,s.pS) < 0;
}

int CString::operator>(const CString& s) const {
	 return strcmp(pS,s.pS) > 0;
}

int CString::operator>=(const CString& s) const {
	 return strcmp(pS,s.pS) >= 0;
}

int CString::operator<=(const CString& s) const {
	 return strcmp(pS,s.pS) <= 0;
}

// -----------  misc -----------------


void CString::Print() const {					// Debug -- Print
	cout << "CString class at ";
	cout << hex << (int)(this) << "\n";
	cout << "length "<< dec << szS << "\n";
	pS?cout << "value: "<< pS << endl:cout<<"not initialized"<<endl;
}


//------------  Data export ---------

inline
int CString::length() const { return szS; }	// return string length

inline
char * CString::toChar(char * s ) const {		// convert to char *
	strcpy(s,pS);
	return s;
}

inline
	   char CString::operator [] ( int sub) const { 	// subscript
		assert( sub >=0 && sub < szS);
		return pS[sub];
}

//**************  Substring Operator () *****************************
CString CString::operator()(int index, int length) const {
	assert(index>=0 && index<szS && length>=0);

	if ((length==0) || (index+length > szS))
		length=szS-index+1;
	CString temp(length);
	strncpy(temp.pS, &pS[index],length);
	temp.pS[length]='\0';
	return temp;
}

CString CString::operator()(int index) const { 			// length-> end of string
	assert(index>=0 && index<szS);
   int length=szS-index+1;
	CString temp(length);
	strncpy(temp.pS, &pS[index],length);
	temp.pS[length]='\0';
	return temp;
}



//-------------  ostream << print out string (friend) ----------------
ostream& operator << (ostream & os, CString& str)
{
	if (!str.szS)
		os << "<empty>";
	else
		os << str.pS;
	return os;
}

//-------------  istream << input string (friend) ---------------------
istream& operator >> (istream & is, CString& str)
{
	char * ptemp = new char[str.szS+1]; // get space for new string
	is.width(str.szS);						// set maximum length
	is>>ptemp;									// input into temp area
	str = ptemp;								// convert to string
	delete[] ptemp;							// free temp area
	return is;
}


//---------  Iterators -------------------------------------------------
CStringIterator::CStringIterator(CString& s) :   // constructor
		pStr(&s), where(0) {}

char CStringIterator::operator() () { 				 // iterator
	if (where < pStr->szS)
		return  pStr->pS[where++];
	return where=0;
}

