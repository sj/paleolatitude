/*
 * PaleoLatitudeTest.h
 *
 *  Created on: 20 Aug 2014
 *      Author: bas
 */

#ifndef PALEOLATITUDETEST_H_
#define PALEOLATITUDETEST_H_

#include "../src/gtest-includes.h"

using namespace std;

class PaleoLatitudeTest : public ::testing::Test {
public:
	static void testLocation(double lat, double lon, double pl_lower, double pl_upper);

};

#endif /* PALEOLATITUDETEST_H_ */
