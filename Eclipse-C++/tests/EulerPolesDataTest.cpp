/*
 * EulerPolesDataTest.cpp
 *
 *  Created on: 4 Jul 2014
 *      Author: bas
 */

#include "EulerPolesDataTest.h"
#include "../src/paleo_latitude/PLEulerPolesReconstructions.h"

#include <vector>

#include "../src/paleo_latitude/PLPlate.h"
using namespace std;
using namespace paleo_latitude;

const string EulerPolesDataTest::TORSVIK_CSV = "data/euler-torsvik-2012.csv";

// All possible CSV files with Euler pole rotation parameters. Note: order of these
// files should match the order in PolarWanderPathsDataTest.h!
const array<string, 3> EulerPolesDataTest::CSV_FILES =  {
		TORSVIK_CSV,
		"data/euler-besse-courtillot-2002.csv",
		"data/euler-kent-irving-2010.csv"
};

TEST_F(EulerPolesDataTest, TestValueBounds){
	for (string csvfile : EulerPolesDataTest::CSV_FILES){
		PLEulerPolesReconstructions* euler = PLEulerPolesReconstructions::readFromFile(csvfile);

		auto entries = euler->getEntries();
		for (auto entry : entries){
			ASSERT_LE(0, entry.age) << "unexpected negative age";
			ASSERT_GE(1000, entry.age) << "age more than 1000 million years";

			const string lat_err = "latitude outside [-90,90] in " + csvfile + " - did latitude and longitude get mixed up?";
			ASSERT_LE(-90.01, entry.latitude) << lat_err;
			ASSERT_GE(90.01, entry.latitude) << lat_err;

			const string lon_err = "longitude outside [-180,180] in " + csvfile;
			ASSERT_LE(-180.01, entry.longitude) << lon_err;
			ASSERT_GE(180.01, entry.longitude) << lon_err;

			// Guess: rotation never outside [-90,90]. Not sure whether this might change in the future?
			const string rot_err = "rotation outside bounds [-120,120] in " + csvfile + " - this is unlikely, but update tests if all right";
			ASSERT_LE(-120.01, entry.rotation) << rot_err;
			ASSERT_GE(120.01, entry.rotation) << rot_err;
		}
	}
}


TEST_F(EulerPolesDataTest, TestRelevantAges1){
	// This test assumes that the following ages are available: {10, 20, 30, 40, 50, 60}
	PLEulerPolesReconstructions* ep = PLEulerPolesReconstructions::readFromFile(EulerPolesDataTest::TORSVIK_CSV);
	vector<Coordinate>* dummy_vec = new vector<Coordinate>();
	PLPlate* plate = new PLPlate(101, "North America", dummy_vec);

	vector<unsigned int> rel_ages = ep->getRelevantAges(plate, 15, 55);
	vector<unsigned int> expected = {10, 20, 30, 40, 50, 60};

	ASSERT_EQ(expected.size(), rel_ages.size());
	for(unsigned int i = 0; i < expected.size(); i++) ASSERT_EQ(expected[i], rel_ages[i]);

	delete plate;
	delete ep;
}

TEST_F(EulerPolesDataTest, TestRelevantAges2){
	// This test assumes that the following ages are available: {10, 20, 30, 40, 50, 60}
	PLEulerPolesReconstructions* ep = PLEulerPolesReconstructions::readFromFile(EulerPolesDataTest::TORSVIK_CSV);
	vector<Coordinate>* dummy_vec = new vector<Coordinate>();
	PLPlate* plate = new PLPlate(101, "North America", dummy_vec);

	vector<unsigned int> rel_ages = ep->getRelevantAges(plate, 40, 60);
	vector<unsigned int> expected = {40,50,60};

	ASSERT_EQ(expected.size(), rel_ages.size());
	for(unsigned int i = 0; i < expected.size(); i++) ASSERT_EQ(expected[i], rel_ages[i]);

	delete plate;
	delete ep;
}



TEST_F(EulerPolesDataTest, TestRelevantAges3){
	// This test assumes that the following ages are available: {10, 20, 30, 40, 50, 60}
	PLEulerPolesReconstructions* ep = PLEulerPolesReconstructions::readFromFile(EulerPolesDataTest::TORSVIK_CSV);
	vector<Coordinate>* dummy_vec = new vector<Coordinate>();
	PLPlate* plate = new PLPlate(101, "North America", dummy_vec);

	vector<unsigned int> rel_ages = ep->getRelevantAges(plate, 50,50);
	vector<unsigned int> expected = {50};

	ASSERT_EQ(expected.size(), rel_ages.size());
	for(unsigned int i = 0; i < expected.size(); i++) ASSERT_EQ(expected[i], rel_ages[i]);

	delete plate;
	delete ep;
}
