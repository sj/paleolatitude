/*
 * PaleoLatitudeTest.cpp
 *
 *  Created on: 20 Aug 2014
 *      Author: bas
 */

#include "EulerPolesDataTest.h"
#include "PaleoLatitudeTest.h"
#include "PlateDataTest.h"
#include "PolarWanderPathsDataTest.h"
#include "../src/paleo_latitude/PLParameters.h"
#include "../src/paleo_latitude/PaleoLatitude.h"

#include <utility>

#include "../src/util/Logger.h"

using namespace std;
using namespace paleo_latitude;

/**
 * This tests the implementation's behaviour regarding unconstrained plates. Should this test fail,
 * then this is caused by (at least) one of three things:
 *
 *  1) bug introduced in the implementation
 *  2) the data no longer specifies the unconstrained plates correctly
 *  3) the plate that is assumed to be unconstrained is no longer unconstrained.
 *
 * In case 3, this test should be changed to another unconstrained plate.
 */
TEST_F(PaleoLatitudeTest, TestUnconstrainedPlate){
	PLParameters* pl_params = new PLParameters();
	pl_params->all_ages = true;
	pl_params->site_latitude = 32;
	pl_params->site_longitude = 70;

	PaleoLatitude pl(pl_params);

	// This will probably print an error - that's fine
	cerr << "This test verifies that working on an unconstrained plate does not cause problems. Expect an error message next." << endl;
	bool compute_result = pl.compute();
	cerr << "An error message should have appeared above. No more errors are expected after this (unless specified otherwise)." << endl;

	ASSERT_FALSE(compute_result) << "Paleolatitude computation succeeded for unconstrained plate";
}

TEST_F(PaleoLatitudeTest, TestParameterValidation){
	PLParameters* pl_params = new PLParameters();
	pl_params->all_ages = true;

	ASSERT_FALSE(pl_params->validate()) << "Parameter validation should have failed due to missing site coordinates";

	pl_params->site_latitude = 32;
	pl_params->site_longitude = 70;
	ASSERT_TRUE(pl_params->validate()) << "Parameter validation should have passed: both all-ages and site coordinates have been specified";

	pl_params->age = 50;
	ASSERT_FALSE(pl_params->validate()) << "Parameter validation should have failed: both all-ages and a specific age requested";

	pl_params->all_ages = false;
	ASSERT_FALSE(pl_params->validate()) << "Parameter validation should have failed: age specified, but without min-age and max-age, or age-pm";

	pl_params->age_min = 70;
	pl_params->age_max = 110;
	ASSERT_FALSE(pl_params->validate()) << "Parameter validation should have failed: invalid min-age/max-age specified";

	pl_params->age_min = 40;
	ASSERT_TRUE(pl_params->validate()) << "Parameter validation should have passed: valid age, age-min, and age-max specified";

	pl_params->age_min = -500;
	ASSERT_FALSE(pl_params->validate()) << "Parameter validation should have failed: invalid age specified";

	pl_params->age_pm = 60;
	ASSERT_FALSE(pl_params->validate()) << "Parameter validation should have failed: both age-pm and age-min/age-max specified";

	pl_params->age_min = -999;
	pl_params->age_max = -999;
	ASSERT_TRUE(pl_params->validate()) << "Parameter validation should have passed: valid age and age-pm specified";
}


/**
 * Verifies whether the data points are returned in the correct order (i.e., sorted by age, most recent data point first)
 */
TEST_F(PaleoLatitudeTest, TestDateOrder){
	PLParameters* params = new PLParameters();
	params->age = 51.5;
	params->age_min = 48;
	params->age_max = 55;
	params->site_latitude = 53.5;
	params->site_longitude = 73.5;
	params->all_ages = false;

	PaleoLatitude pl(params);
	pl.compute();

	auto relEntries = pl.getRelevantPaleolatitudeEntries();
	for (unsigned int i = 0; i < relEntries.size() - 1; i++){ // deliberate size() - 1: always looking forward 1 item
		ASSERT_LE(relEntries[i].age_years, relEntries[i+1].age_years) << "Paleolatitude entries returned in wrong order";
	}

	// Just make sure that there is at least one entry to the left, and one to the right of the bounds
	ASSERT_LE(relEntries[0].getAgeInMYR(), params->age_min) << "Computation did not return entry on/outside lower age bound of " << params->age_min;
	ASSERT_GE(relEntries[relEntries.size() - 1].getAgeInMYR(), params->age_max) << "Computation did not return entry on/outside lower age bound of " << params->age_max;


	delete params;
}

size_t PaleoLatitudeTest::TestEntry::numColumns() const {
	return 13;
}

void PaleoLatitudeTest::TestEntry::set(unsigned int column, string value, string filename, unsigned int line_no){
	CSVFileData<TestEntry>::StringEntry::set(column, value, filename, line_no);

	if (value.empty()) return;
	if (column == TEST_NAME) this->parseString(value, test_name);
	if (column == LATITUDE) this->parseString(value, latitude);
	if (column == LONGITUDE) this->parseString(value, longitude);
	if (column == EXPECTED_PLATE_ID) this->parseString(value, expected_plate_id);
	if (column == EXPECTED_PLATE_NAME) this->parseString(value, expected_plate_name);
	if (column == TEST_AGE) this->parseString(value, test_age);
	if (column == TEST_AGE_LOWERBOUND) this->parseString(value, test_age_lowerbound);
	if (column == TEST_AGE_UPPERBOUND) this->parseString(value, test_age_upperbound);
	if (column == EULER_DATA) this->parseString(value, euler_data);
	if (column == APWP_DATA) this->parseString(value, apwp_data);
	if (column == EXPECTED_PALEOLATITUDE) this->parseString(value, expected_paleolatitude);
	if (column == EXPECTED_PALEOLATITUDE_LOWERBOUND) this->parseString(value, expected_paleolatitude_lowerbound);
	if (column == EXPECTED_PALEOLATITUDE_UPPERBOUND) this->parseString(value, expected_paleolatitude_upperbound);
}

TEST_F(PaleoLatitudeTest, TestLocationsFromCSV){
	// Open 'paleolatitude-test-data.csv': specifies a number of locations,
	// their expected plates, and their expected paleolatitudes
	const string csvfile = "data/paleolatitude-test-data.csv";
	CSVFileData<TestEntry>* csvdata = new CSVFileData<TestEntry>();
	csvdata->parseFile(csvfile);

	unsigned int counter = 0;
	for (const TestEntry& entry : csvdata->getEntries()){
		counter++;

		// Test plate data first
		cout << "Test location " << counter << " (line " << entry.getLineNo() << " of " << csvfile << "): " << entry.test_name << " (" << entry.latitude << "," << entry.longitude << ")..." << endl;

		unsigned int computed_plate_id = 0;
		string computed_plate_name = "";
		PlateDataTest::testLocation(entry.latitude, entry.longitude, entry.expected_plate_id, entry.expected_plate_name, computed_plate_id, computed_plate_name);

		if (entry.empty(TestEntry::TEST_AGE_LOWERBOUND) && entry.empty(TestEntry::TEST_AGE_UPPERBOUND) && entry.empty(TestEntry::TEST_AGE)){
			// Test does not specify age data - do not test paleolatitude
			cout << "  -> plate: " << computed_plate_name << " (" << computed_plate_id << "). Paleolatitude not computed as part of test." << endl;
			continue;
		}


		vector<string> euler_data_csvs;
		vector<string> apwp_data_csvs;
		if (entry.empty(TestEntry::APWP_DATA) && entry.empty(TestEntry::EULER_DATA)){
			// No specific Euler/APWP data specified: use all
			euler_data_csvs.resize(EulerPolesDataTest::CSV_FILES.size());
			apwp_data_csvs.resize(PolarWanderPathsDataTest::CSV_FILES.size());
			copy(EulerPolesDataTest::CSV_FILES.begin(), EulerPolesDataTest::CSV_FILES.end(), euler_data_csvs.begin());
			copy(PolarWanderPathsDataTest::CSV_FILES.begin(), PolarWanderPathsDataTest::CSV_FILES.end(), apwp_data_csvs.begin());
		} else if (!entry.empty(TestEntry::APWP_DATA) && !entry.empty(TestEntry::EULER_DATA)){
			// Only test using the specified Euler/APWP data
			euler_data_csvs.push_back(entry.euler_data);
			apwp_data_csvs.push_back(entry.apwp_data);
		} else {
			FAIL() << "Line " << entry.getLineNo() << " of " << csvfile << " specifies only one of (Euler data, APWP data): expecting either both or neither" << endl;
		}

		for (unsigned int i = 0; i < euler_data_csvs.size(); i++){

			// Test paleolatitude computation
			PLParameters* pl_params = new PLParameters();
			pl_params->site_latitude = entry.latitude;
			pl_params->site_longitude = entry.longitude;
			pl_params->input_apwp_csv = apwp_data_csvs[i];
			pl_params->input_euler_rotation_csv = euler_data_csvs[i];

			if (!entry.empty(TestEntry::TEST_AGE_LOWERBOUND)) pl_params->age_min = entry.test_age_lowerbound;
			if (!entry.empty(TestEntry::TEST_AGE_UPPERBOUND)) pl_params->age_max = entry.test_age_upperbound;
			if (!entry.empty(TestEntry::TEST_AGE)) pl_params->age = entry.test_age;

			PaleoLatitude pl(pl_params);
			pl.compute();

			PaleoLatitude::PaleoLatitudeEntry res = pl.getPaleoLatitude();
			stringstream res_details;
			res_details << "plate: " << computed_plate_name << " (" << computed_plate_id << ")";

			if (!entry.empty(TestEntry::EXPECTED_PALEOLATITUDE)){
				ASSERT_NEAR(entry.expected_paleolatitude, res.palat, 0.001) << "Unexpected paleolatitude (using " << entry.euler_data << ", " << entry.apwp_data << ")";
				res_details << ", paleolatitude: " << res.palat;
			}

			if (!entry.empty(TestEntry::EXPECTED_PALEOLATITUDE_LOWERBOUND)){
				ASSERT_LE(entry.expected_paleolatitude_lowerbound, res.palat_min) << "Unexpected location paleolatitude lower bound (using " << entry.euler_data << ", " << entry.apwp_data << ")";
				res_details << ", lower bound: " << res.palat_min;
			}

			if (!entry.empty(TestEntry::EXPECTED_PALEOLATITUDE_UPPERBOUND)){
				ASSERT_GE(entry.expected_paleolatitude_upperbound, res.palat_max) << "Unexpected location paleolatitude upper bound (using " << entry.euler_data << ", " << entry.apwp_data << ")";
				res_details << ", upper bound: " << res.palat_max;
			}

			res_details << " (using " << pl_params->input_euler_rotation_csv << " and " << pl_params->input_apwp_csv << ")";

			cout << "  -> " << res_details.str() << endl;

			delete pl_params;
		}
	}

	delete csvdata;
}
