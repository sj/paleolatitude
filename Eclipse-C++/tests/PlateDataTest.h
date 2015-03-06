/*
 * PlateDataTest.h
 *
 *  Created on: 29 Jun 2014
 *      Author: bas
 */

#ifndef PLATEDATATEST_H_
#define PLATEDATATEST_H_

#include <string>
#include "../src/gtest-includes.h"

using namespace std;

class PlateDataTest : public ::testing::Test {

public:
	static void testLocation(double lat, double lon, unsigned int expected_plate_id, string expected_plate_name);
};


#endif /* PLATEDATATEST_H_ */
