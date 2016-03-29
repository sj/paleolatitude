/*
 * PaleoLatitudeTest.cpp
 *
 *  Created on: 20 Aug 2014
 *      Author: bas
 */

#include "EulerPolesDataTest.h"
#include "PaleoLatitudeTest.h"
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

void PaleoLatitudeTest::testLocation(double lat, double lon, double expected_pl_lower, double expected_pl_upper){
	testLocation(lat, lon, 0, 99999, expected_pl_lower, expected_pl_upper);
}

void PaleoLatitudeTest::testLocation(double lat, double lon, unsigned int age_min, unsigned int age_max, double expected_pl_lower, double expected_pl_upper){
	PLParameters* pl_params = new PLParameters();
	pl_params->site_latitude = lat;
	pl_params->site_longitude = lon;
	//pl_params->age = 100;
	pl_params->age_min = 0;
	pl_params->age_max = age_max;

	for (unsigned int i = 0; i < PolarWanderPathsDataTest::CSV_FILES.size(); i++){
		const string euler_csvfile = EulerPolesDataTest::CSV_FILES[i];
		const string apwp_csvfile = PolarWanderPathsDataTest::CSV_FILES[i];

		pl_params->input_apwp_csv = apwp_csvfile;
		pl_params->input_euler_rotation_csv = euler_csvfile;

		Logger::debug << "Testing paleolatitude computation for location (lat=" << lat << ",lon=" << lon << ") using " << euler_csvfile << ", " << apwp_csvfile << "..." << endl;

		PaleoLatitude pl(pl_params);
		pl.compute();

		PaleoLatitude::PaleoLatitudeEntry res = pl.getPaleoLatitude();

		Logger::debug << "Verifying computed latitude bounds for location (lat=" << lat << ",lon=" << lon << "): bounds [" << res.palat_min << "," << res.palat_max << "] established using " << euler_csvfile << ", " << apwp_csvfile << "..." << endl;
		ASSERT_LE(expected_pl_lower, res.palat_min) << "Unexpected location paleolatitude lower bound (using " << euler_csvfile << ", " << apwp_csvfile << ")";
		ASSERT_GE(expected_pl_upper, res.palat_max) << "Unexpected location paleolatitude upper bound (using " << euler_csvfile << ", " << apwp_csvfile << ")";
	}

}


TEST_F(PaleoLatitudeTest, TestLocationsFromCSV){
	// Open 'paleolatitude-test-data.csv': specifies a number of locations,
	// their expected plates, and their expected paleolatitudes
	CSVFileData<TestEntry> csvdata = new CSVFileData<TestEntry>();
	csvdata->parseFile("data/paleolatitude-test-data.csv");


	FAIL() << "todo";
}

/**
 * The tests below were based on data that was accurate on 20/08/2014. If one of these tests fails,
 * check the implementation, check the data, and update the test if the data turns out to yield a
 * different paleolatitude.
 */

TEST_F(PaleoLatitudeTest, TestCapetown){
	testLocation(-33.925278, 18.423889, 0, 200, -56, -21);
}

TEST_F(PaleoLatitudeTest, TestNorthAmericaUpto260ma){
	// Expected data provided by DvH Feb 2016
	testLocation(45, -100, 0, 260, 10, 63);
}

TEST_F(PaleoLatitudeTest, TestNorthAmerica260until){
	// Expected data provided by DvH Feb 2016
	testLocation(45, -100, 0, 260, 10, 63);
}

TEST_F(PaleoLatitudeTest, TestAntananarivoMadagascar){
	testLocation(-18.933333, 47.516667, 0, 200, -56, -16);
}

TEST_F(PaleoLatitudeTest, TestNewDelhi){
	testLocation(28.613889, 77.208889, 0, 200, -63, 32);
}

TEST_F(PaleoLatitudeTest, TestNuuk){
	testLocation(64.175, -51.738889, 0, 200, 35, 68);
}

TEST_F(PaleoLatitudeTest, TestHonolulu){
	testLocation(21.3, -157.816667, 0, 200, -6, 28);
}

TEST_F(PaleoLatitudeTest, TestAmsterdam){
	testLocation(52.366667, 4.9, 0, 200, 20, 60);
}


