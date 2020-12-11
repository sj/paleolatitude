/*
 * EulerPolesDataTest.h
 *
 *  Created on: 4 Jul 2014
 *      Author: bas
 */

#ifndef EULERPOLESDATATEST_H_
#define EULERPOLESDATATEST_H_

#include "../src/gtest-includes.h"
#include <array>
#include <string>

using namespace std;

class EulerPolesDataTest : public ::testing::Test {

public:
	const static string TORSVIK_VANDERVOO_CSV;
	const static array<string,3> CSV_FILES;
};


#endif /* EULERPOLESDATATEST_H_ */
