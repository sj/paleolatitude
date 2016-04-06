/*
 * PolarWanderPathsDataTest.cpp
 *
 *  Created on: 30 Jun 2014
 *      Author: bas
 */

#include "PolarWanderPathsDataTest.h"
#include "../src/paleo_latitude/PLPolarWanderPaths.h"
#include "../src/paleo_latitude/PLEulerPolesReconstructions.h"
#include "EulerPolesDataTest.h"
#include "../src/util/Exception.h"
#include <vector>
#include <array>
#include <sstream>

using namespace std;
using namespace paleo_latitude;


const string PolarWanderPathsDataTest::TORSVIK_VANDERVOO_CSV = "data/apwp-torsvik-2012-vandervoo-2015.csv";

// Make sure the order of these files matches the order of files in EulerPolesDataTest.h:
const array<string, 3> PolarWanderPathsDataTest::CSV_FILES = {
		TORSVIK_VANDERVOO_CSV,
		"data/apwp-besse-courtillot-2002-vandervoo-2015.csv",
		"data/apwp-kent-irving-2010-vandervoo-2015.csv"
};


TEST_F(PolarWanderPathsDataTest, Test10ma701){
	PLPolarWanderPaths* p = PLPolarWanderPaths::readFromFile(PolarWanderPathsDataTest::TORSVIK_VANDERVOO_CSV);
	const PLPolarWanderPaths::PWPEntry* e = p->getEntry(701, 10);
	ASSERT_GE(1.9, e->a95);
	ASSERT_LE(0, e->a95);
}

TEST_F(PolarWanderPathsDataTest, Test200ma701){
	PLPolarWanderPaths* p = PLPolarWanderPaths::readFromFile(PolarWanderPathsDataTest::TORSVIK_VANDERVOO_CSV);
	const PLPolarWanderPaths::PWPEntry* e = p->getEntry(701, 200);
	ASSERT_LE(0, e->a95);


}

TEST_F(PolarWanderPathsDataTest, Test9999ma701){
	PLPolarWanderPaths* p = PLPolarWanderPaths::readFromFile(PolarWanderPathsDataTest::TORSVIK_VANDERVOO_CSV);
	ASSERT_THROW(p->getEntry(701, 9999), Exception);
}

/**
 * Tests whether all APWPs referred to from the Euler rotation data are available,
 * and whether there is Euler rotation data for all APWPs
 */
TEST_F(PolarWanderPathsDataTest, TestEulerAPWPDataConsistency){
	stringstream all_errors;

	for (unsigned int i = 0; i < PolarWanderPathsDataTest::CSV_FILES.size(); i++){
		const string apwp_file = PolarWanderPathsDataTest::CSV_FILES[i];
		const string euler_file = EulerPolesDataTest::CSV_FILES[i];

		// Read Euler rotation data, and check that every rotation specifies a
		// relative plate that exists in the APWP data for that age
		PLEulerPolesReconstructions* epr = PLEulerPolesReconstructions::readFromFile(euler_file);
		PLPolarWanderPaths* apwp = PLPolarWanderPaths::readFromFile(apwp_file);

		auto euler_entries = epr->getAllEntries();

		for (auto euler_entry : euler_entries){
			const unsigned int rel_plate_id = euler_entry.rotation_rel_to_plate_id;
			const unsigned int age = euler_entry.age;

			try {
				apwp->getEntry(rel_plate_id, age);
			} catch (Exception& ex){
				stringstream this_error;
				this_error << "Euler rotation data in '" << euler_file << "' (line " << euler_entry.getLineNo() << ") requires an apparent polar wander path for plate " << rel_plate_id << " and age " << age << ", but '" << apwp_file << "' does not contain that data." << endl;

				cerr << this_error.str();
				all_errors << this_error.str();
			}
		}

		auto apwp_entries = apwp->getAllEntries();
		for (auto apwp_entry : apwp_entries){
			const unsigned int plate_id = apwp_entry.plate_id;
			const unsigned int age = apwp_entry.age;

			try {
				epr->getEntries(plate_id, age);
			} catch (Exception& ex){
				cerr << "Warning: apparent polar wander path available for plate ID " << plate_id << " and age " << age << " in '" << apwp_file << "' (line " << apwp_entry.getLineNo() << ", but this data is not referred to from Euler rotation data in '" << euler_file << "', so is never used?" << endl;
			}
		}
	}

	if (!all_errors.str().empty()){
		FAIL() << all_errors.str();
	}
}


TEST_F(PolarWanderPathsDataTest, TestValueBounds){
	for (string apwp_file : PolarWanderPathsDataTest::CSV_FILES){
		PLPolarWanderPaths* p = PLPolarWanderPaths::readFromFile(apwp_file);

		auto entries = p->getAllEntries();
		for (auto entry : entries){
			ASSERT_LE(0, entry.age) << "unexpected negative age";
			ASSERT_GE(1000, entry.age) << "age more than 1000 million years";

			const string lat_err = "latitude outside [-90,90] - did latitude and longitude get mixed up?";
			ASSERT_LE(-90.01, entry.latitude) << lat_err;
			ASSERT_GE(90.01, entry.latitude) << lat_err;

			const string lon_err = "longitude outside [0,360]";
			ASSERT_LE(0, entry.longitude) << lon_err;
			ASSERT_GE(360, entry.longitude) << lon_err;
		}
	}
}


