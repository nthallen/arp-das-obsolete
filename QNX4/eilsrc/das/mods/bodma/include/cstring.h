#if !defined __CSTRING
#define __CSTRING

#include <iostream.h>
#include <string.h>
class CString {
public:
											// Constructors & deconstructor
	CString( const int ); 			// allocate string with size
	CString( const char *);			// allocate string and init
	CString();							// default allocation
	CString( const CString&);		// copy constructor
	~CString();							// deconstructor

											// concat Operator
	CString& operator+=( const CString& );
	CString& operator+=( const char *);
	CString  operator* ( const CString &s ) const;	// example
											// Assignment
	CString& operator=(const CString&) ;

											// logical operators
	int operator==(const CString&) const;
	int operator!=(const CString&) const;
	int operator< (const CString&) const;
	int operator> (const CString&) const;
	int operator>=(const CString&) const;
	int operator<=(const CString&) const;

											// friends
	friend CString  operator+ (const CString&, const CString&);
	friend ostream& operator << (ostream & os, CString& str);
	friend istream& operator >> (istream & is, CString& str);

											// data output
	inline int length() const;				// return length
	inline char* toChar(char *) const;	// Convert
	CString operator()(int,int) const;	// return a substring
   CString operator()(int) const;      // return a substring to end
	inline
	char  operator [] ( int) const;     // return a single character
	void Print () const;						// print string

											// iterator
	friend class CStringIterator;


protected:
	int szS;
	char *pS;
};


class CStringIterator {
public:
	CStringIterator(CString& s);
	char operator() ();
private:
	CString *pStr;
	int	where;
};


typedef CString string;
#endif
