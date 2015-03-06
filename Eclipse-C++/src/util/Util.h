/*
 * Util.h
 *
 *  Created on: 30 Jun 2014
 *      Author: bas
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <sstream>

using namespace std;

namespace paleo_latitude {

class Util {
public:
	template<class T> static bool string_to_something(const string& str, T& result){
		istringstream iss(str);
		T tmp;
		if (!(iss >> tmp)) return false;

		result = tmp;
		return true;
	}

	static bool string_ends_with(const string& some_string, const string& suffix);

	static bool double_eq(const double& a, const double& b);

	constexpr static double DOUBLE_COMPARISON_EPSILON = 0.0000000001;
};

};

#endif /* UTIL_H_ */
