/*
 * PolarWanderPathsDataTest.cpp
 *
 *  Created on: 30 Jun 2014
 *      Author: bas
 */

#include "PolarWanderPathsDataTest.h"
#include "../src/paleo_latitude/PLPolarWanderPaths.h"
#include "../src/util/Exception.h"
#include <vector>

using namespace std;
using namespace paleo_latitude;


const string PolarWanderPathsDataTest::TORSVIK_CSV = "data/apwp-701-torsvik-2012.csv";

// Make sure the order of these files matches the order of files in EulerPolesDataTest.h:
const array<string, 3> PolarWanderPathsDataTest::CSV_FILES = {
		TORSVIK_CSV,
		"data/apwp-701-besse-courtillot-2002.csv",
		"data/apwp-701-kent-irving-2010.csv"
};


TEST_F(PolarWanderPathsDataTest, Test10miy){
	PLPolarWanderPaths* p = PLPolarWanderPaths::readFromFile(PolarWanderPathsDataTest::TORSVIK_CSV);
	PLPolarWanderPaths::PWPEntry e = p->getEntry(10);
	ASSERT_GE(1.9, e.a95);
	ASSERT_LE(0, e.a95);
}

TEST_F(PolarWanderPathsDataTest, Test200miy){
	PLPolarWanderPaths* p = PLPolarWanderPaths::readFromFile(PolarWanderPathsDataTest::TORSVIK_CSV);
	PLPolarWanderPaths::PWPEntry e = p->getEntry(200);
	ASSERT_LE(0, e.a95);


}

TEST_F(PolarWanderPathsDataTest, Test9999miy){
	PLPolarWanderPaths* p = PLPolarWanderPaths::readFromFile(PolarWanderPathsDataTest::TORSVIK_CSV);
	ASSERT_THROW(p->getEntry(9999), Exception);
}



TEST_F(PolarWanderPathsDataTest, TestValueBounds){
	for (string apwp_file : PolarWanderPathsDataTest::CSV_FILES){
		PLPolarWanderPaths* p = PLPolarWanderPaths::readFromFile(apwp_file);

		auto entries = p->getEntries();
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


