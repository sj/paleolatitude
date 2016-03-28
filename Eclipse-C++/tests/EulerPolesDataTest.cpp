/*
 * EulerPolesDataTest.cpp
 *
 *  Created on: 4 Jul 2014
 *      Author: bas
 */

#include "EulerPolesDataTest.h"
#include "../src/paleo_latitude/PLEulerPolesReconstructions.h"

#include <vector>
#include <set>

#include "../src/paleo_latitude/PLPlate.h"
using namespace std;
using namespace paleo_latitude;

const string EulerPolesDataTest::TORSVIK_VANDERVOO_CSV = "data/euler-torsvik-2012-vandervoo-2015.csv";

// All possible CSV files with Euler pole rotation parameters. Note: order of these
// files should match the order in PolarWanderPathsDataTest.h!
const array<string, 3> EulerPolesDataTest::CSV_FILES =  {
		TORSVIK_VANDERVOO_CSV,
		"data/euler-besse-courtillot-2002.csv",
		"data/euler-kent-irving-2010.csv"
};

TEST_F(EulerPolesDataTest, TestValueBounds){
	for (string csvfile : EulerPolesDataTest::CSV_FILES){
		PLEulerPolesReconstructions* euler = PLEulerPolesReconstructions::readFromFile(csvfile);

		auto entries = euler->getAllEntries();
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

TEST_F(EulerPolesDataTest, TestMultipleRelativePlateIdsForAge){
	for (string csvfile : EulerPolesDataTest::CSV_FILES){
		PLEulerPolesReconstructions* euler = PLEulerPolesReconstructions::readFromFile(csvfile);

		set<unsigned int> plate_ids = euler->getPlateIds();
		for (unsigned int plate_id : plate_ids){
			vector<const PLEulerPolesReconstructions::EPEntry*> entries = euler->getEntries(plate_id);
			sort(entries.begin(), entries.end(), PLEulerPolesReconstructions::EPEntry::compareByAge);

			const PLEulerPolesReconstructions::EPEntry* prev = NULL;
			const PLEulerPolesReconstructions::EPEntry* crossover_at = NULL;
			for (const PLEulerPolesReconstructions::EPEntry* entry : entries){
				if (prev != NULL && prev->age == entry->age){
					// Duplicate age entry. This must be the cross-over point between two
					// reference plates. Make sure that there is only a single cross-over
					// point - this is what the code assumes. If this changes
					// (i.e., multiple cross-over points between relative plates), then
					// all code needs double-checking and amending.
					// Note that this test is mostly designed to make sure that the overlap
					// between two methods of computing the paleolatitude is not more than
					// one age data point.
					if (crossover_at != NULL){
						// Already found a cross-over point
						FAIL() << "Multiple duplicate Euler pole rotation data for plate " << plate_id << " in '" << csvfile << "', only one duplicate allowed per plate: first duplicate at age " << crossover_at->age << " (plate " << crossover_at->plate_id << ", rotation relative to: " << crossover_at->rotation_rel_to_plate_id << ", line " << crossover_at->getLineNo() << "), another one at " << entry->age << " (plate " << entry->plate_id << ", rotation relative to: " << entry->rotation_rel_to_plate_id << ", line " << entry->getLineNo() << ")";
					}

					crossover_at = entry;
				}

				prev = entry;
			} // end foreach entry
		} // end foreach plate ID

		delete euler;
	} // end foreach CSV file
}


TEST_F(EulerPolesDataTest, TestRelevantAges1){
	// This test assumes that the following ages are available: {10, 20, 30, 40, 50, 60}
	PLEulerPolesReconstructions* ep = PLEulerPolesReconstructions::readFromFile(EulerPolesDataTest::TORSVIK_VANDERVOO_CSV);
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
	PLEulerPolesReconstructions* ep = PLEulerPolesReconstructions::readFromFile(EulerPolesDataTest::TORSVIK_VANDERVOO_CSV);
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
	PLEulerPolesReconstructions* ep = PLEulerPolesReconstructions::readFromFile(EulerPolesDataTest::TORSVIK_VANDERVOO_CSV);
	vector<Coordinate>* dummy_vec = new vector<Coordinate>();
	PLPlate* plate = new PLPlate(101, "North America", dummy_vec);

	vector<unsigned int> rel_ages = ep->getRelevantAges(plate, 50,50);
	vector<unsigned int> expected = {50};

	ASSERT_EQ(expected.size(), rel_ages.size());
	for(unsigned int i = 0; i < expected.size(); i++) ASSERT_EQ(expected[i], rel_ages[i]);

	delete plate;
	delete ep;
}
