/*
 * PolarWanderPathsDataTest.cpp
 *
 *  Created on: 30 Jun 2014
 *      Author: bas
 */

#include "UtilTest.h"
#include "../src/util/Util.h"
#include <vector>
#include <array>
#include <sstream>

using namespace std;
using namespace paleo_latitude;

TEST_F(UtilTest, TestStringToSomething){
	string input = "Something with a space";
	string output_str;
	Util::string_to_something(input, output_str);
	ASSERT_EQ(input, output_str);

	input = "5";
	unsigned int output_uint;
	Util::string_to_something(input, output_uint);
	ASSERT_EQ(5, output_uint);
}
