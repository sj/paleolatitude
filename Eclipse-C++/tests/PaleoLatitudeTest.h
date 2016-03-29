/*
 * PaleoLatitudeTest.h
 *
 *  Created on: 20 Aug 2014
 *      Author: bas
 */

#ifndef PALEOLATITUDETEST_H_
#define PALEOLATITUDETEST_H_

#include "../src/gtest-includes.h"
#include "../src/util/CSVFileData.h"
using namespace std;
using namespace paleo_latitude;

class PaleoLatitudeTest : public ::testing::Test {
public:
	struct TestEntry : public CSVFileData<TestEntry>::Entry {
		TestEntry(CSVFileData<TestEntry>* parent, unsigned int line_no) : CSVFileData<TestEntry>::Entry(parent, line_no){}
		void set(unsigned int col_index, const string& value, const string& filename, unsigned int lineno) override;
		unsigned int numColumns() const override;


	};

	static void testLocation(double lat, double lon, double pl_lower, double pl_upper);
	static void testLocation(double lat, double lon, unsigned int min_age, unsigned int max_age, double pl_lower, double pl_upper);
};

#endif /* PALEOLATITUDETEST_H_ */
