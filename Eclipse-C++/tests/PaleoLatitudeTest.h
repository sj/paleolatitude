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
	struct TestEntry : public CSVFileData<TestEntry>::StringEntry {
	public:
		TestEntry(CSVFileData<TestEntry>* parent, unsigned int line_no) : CSVFileData<TestEntry>::StringEntry(parent, line_no){}

		size_t numColumns() const;
		void set(unsigned int column, const string value, const string filename, unsigned int lineno) override;

		string test_name;
		double latitude = 0;
		double longitude = 0;
		unsigned int expected_plate_id = 0;

	private:
		enum columns_t : unsigned int {TEST_NAME = 0, LATITUDE = 1, LONGITUDE = 2, EXPECTED_PLATE_ID = 3};
	};

	static void testLocation(double lat, double lon, double pl_lower, double pl_upper);
	static void testLocation(double lat, double lon, unsigned int min_age, unsigned int max_age, double pl_lower, double pl_upper);
};

#endif /* PALEOLATITUDETEST_H_ */
