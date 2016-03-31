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
		string expected_plate_name;
		unsigned int test_age = 0;
		unsigned int test_age_lowerbound = 0;
		unsigned int test_age_upperbound = 0;
		string euler_data;
		string apwp_data;
		double expected_paleolatitude = 0;
		double expected_paleolatitude_lowerbound = 0;
		double expected_paleolatitude_upperbound = 0;

	private:
		FRIEND_TEST(PaleoLatitudeTest, TestLocationsFromCSV);
		enum columns_t : unsigned int {
			TEST_NAME = 0, LATITUDE = 1, LONGITUDE = 2, EXPECTED_PLATE_ID = 3,
			EXPECTED_PLATE_NAME = 4, TEST_AGE = 5, TEST_AGE_LOWERBOUND = 6, TEST_AGE_UPPERBOUND = 7,
			EULER_DATA = 8, APWP_DATA = 9, EXPECTED_PALEOLATITUDE = 10, EXPECTED_PALEOLATITUDE_LOWERBOUND = 11,
			EXPECTED_PALEOLATITUDE_UPPERBOUND = 12
		};
	};

	static void testLocation(double lat, double lon, double pl_lower, double pl_upper);
	static void testLocation(double lat, double lon, unsigned int min_age, unsigned int max_age, double pl_lower, double pl_upper);
};

#endif /* PALEOLATITUDETEST_H_ */
