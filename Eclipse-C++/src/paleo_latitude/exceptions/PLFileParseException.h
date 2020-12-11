/*
 * PLFileParseException.h
 *
 *  Created on: 3 Jul 2014
 *      Author: Sebastiaan van Schaik
 */

#ifndef PLFILEPARSEEXCEPTION_H_
#define PLFILEPARSEEXCEPTION_H_

#include <string>
#include "../../util/Exception.h"

using namespace std;

namespace paleo_latitude {

class PLFileParseException : public Exception {
	public:
		PLFileParseException(): Exception(){}
		PLFileParseException(string msg) : Exception(msg){}
};

};



#endif /* PLFILEPARSEEXCEPTION_H_ */
