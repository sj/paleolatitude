/*
 * Util.cpp
 *
 *  Created on: 30 Jun 2014
 *      Author: bas
 */

#include "Util.h"
#include <sstream>
#include <cmath>

using namespace paleo_latitude;
using namespace std;

bool Util::string_ends_with(const string& some_string, const string& suffix){
	if (some_string.length() < suffix.length()) return false;

	return (some_string.compare(some_string.length() - suffix.length(), suffix.length(), suffix) == 0);
}

bool Util::double_eq(const double& a, const double& b){
	if (a == b) return true;
	const double diff = (fabs(a-b));

	// Test absolute error:
	if (diff <= DOUBLE_COMPARISON_EPSILON) return true;

	// Do test with relative error (note that if a == 0 and b == 0, this would be
	// a division-by-zero case, but this case is caught by testing for absolute equality)
	return (diff / (fabs(a) + fabs(b)) < DOUBLE_COMPARISON_EPSILON);
}

