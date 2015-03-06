/*
 * Exception.cpp
 *
 *  Created on: 29 Mar 2011
 *      Author: Sebastiaan J. van Schaik
 */

#include "Exception.h"

using namespace paleo_latitude;

Exception::Exception() : _msg(""){}

Exception::Exception(string msg) : _msg(msg){}

Exception::~Exception() throw () {}


Exception& Exception::operator<<(StandardEndLine manip)
{
	stringstream ss;
	ss << _msg << endl;

	_msg = ss.str();
	return *this;
}

const char* Exception::what() const noexcept {
	return _msg.c_str();
}
