/*
 * Exception.h
 *
 *  Created on: 29 Mar 2011
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <exception>
#include <string>
#include <ostream>
#include <sstream>
using namespace std;

namespace paleo_latitude {

class Exception: public exception {
public:
	Exception();
	Exception(string msg);

	virtual ~Exception() throw();

	virtual const char* what() const noexcept override;

	template <class SomeType>
	Exception& operator<<(SomeType val){
		stringstream ss;
		ss << _msg << val;
		_msg = ss.str();
		return *this;
	}

	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
	typedef CoutType& (*StandardEndLine)(CoutType&);
	Exception& operator<<(StandardEndLine manip);

private:
	string _msg;

};

};

#endif /* EXCEPTION_H */
