/*
 * PolarWanderPathsDataTest.h
 *
 *  Created on: 30 Jun 2014
 *      Author: bas
 */

#ifndef POLARWANDERPATHSDATATEST_H_
#define POLARWANDERPATHSDATATEST_H_

#include <string>
#include "../src/gtest-includes.h"
#include <array>

using namespace std;

class PolarWanderPathsDataTest : public ::testing::Test {

public:
	const static string TORSVIK_CSV;
	const static array<string, 3> CSV_FILES;
};

#endif /* POLARWANDERPATHSDATATEST_H_ */
