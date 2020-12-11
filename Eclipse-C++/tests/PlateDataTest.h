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
#include "../src/paleo_latitude/PLPlates.h"
#include "../src/util/CSVFileData.h"

using namespace std;
using namespace paleo_latitude;

class PlateDataTest : public ::testing::Test {

public:
	static void testLocation(double lat, double lon, unsigned int expected_plate_id, string expected_plate_name, unsigned int& computed_plate_id, string& computed_plate_name);
	static void testLocation(double lat, double lon, unsigned int expected_plate_id, string expected_plate_name);

protected:
	struct ExpectedPlatesEntry : public CSVFileData<ExpectedPlatesEntry>::Entry {
		ExpectedPlatesEntry(CSVFileData<ExpectedPlatesEntry>* parent, unsigned int line_no) : CSVFileData<ExpectedPlatesEntry>::Entry(parent, line_no){}
		void set(unsigned int col_index, const string value, const string filename, unsigned int lineno) override;
		size_t numColumns() const override;

		unsigned int plate_id = 0;
		string plate_name = "";
	};
	static void _verifyPlates(const PLPlates* plates, const string plates_file, const CSVFileData<ExpectedPlatesEntry>& expected);

private:
	static string _normalisePlateName(string plate_name);

};


#endif /* PLATEDATATEST_H_ */
