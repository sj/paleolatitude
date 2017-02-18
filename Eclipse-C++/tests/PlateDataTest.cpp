/*
 * PlateDataTest.cpp
 *
 *  Created on: 29 Jun 2014
 *      Author: bas
 */

#include "PlateDataTest.h"

#include "../src/paleo_latitude/PaleoLatitude.h"
#include "../src/paleo_latitude/PLPlate.h"
#include "../src/paleo_latitude/PLPlates.h"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>

using namespace std;
using namespace paleo_latitude;

/**
 * Checks whether all plates are accounted for in the GPML and/or KML data file provided
 * using 'expected-plates.csv'.
 */
TEST_F(PlateDataTest, TestExpectedPlatesInData){
	CSVFileData<ExpectedPlatesEntry> expected_plates(";");
	expected_plates.parseFile("data/expected-plates.csv");

	PLPlates* plp_kml = NULL;
	try {
		string plates_file = "data/plates.kml";
		plp_kml = PLPlates::readFromFile(plates_file);
		_verifyPlates(plp_kml, plates_file, expected_plates);
	} catch (...){} // No worries if file not readable - just use GPML

	PLPlates* plp_gpml = NULL;
	try {
		string plates_file = "data/plates.gpml";
		plp_gpml = PLPlates::readFromFile(plates_file);
		_verifyPlates(plp_gpml, plates_file, expected_plates);
	} catch (...){} // No worries if file not readable - just use KML

	if (plp_kml == NULL && plp_gpml == NULL){
		FAIL() << "Could not read plate data from KML or GPML file?" << endl;
	}

	delete plp_kml;
	delete plp_gpml;
}

void PlateDataTest::_verifyPlates(const PLPlates* plplates, const string plates_file, const CSVFileData<ExpectedPlatesEntry>& expected_plates){
	const vector<const PLPlate*> plates = plplates->getPlates();

	// Check whether all plates have unique IDs. A plate may only be defined twice (with the same ID)
	// if it is in fact the same plate that consists of two different parts
	bool fail = false;
	for (const PLPlate* plate1 : plates){
		for (const PLPlate* plate2 : plates){
			if (plate1->getId() == plate2->getId() && plate1->getName() != plate2->getName()){
				fail = true;
				cerr << "Data in '" << plates_file << "' contains two different plates with identical ID " << plate1->getId() << ": '" << plate1->getName() << "' and '" << plate2->getName() << "'\n";
			}
		}
	}

	// Check whether all expected plates are indeed found in the data file
	for (ExpectedPlatesEntry exp : expected_plates.getEntries()){
		bool matched = false;

		for (const PLPlate* plate : plates){
			if (exp.plate_id == plate->getId()){
				if (exp.plate_name != plate->getName()){
					fail = true;
					cerr << "Plate with ID " << exp.plate_id << " was expected to have name '" << exp.plate_name << "', but is referred to as '" << plate->getName() << "' in '" << plates_file << "'\n";
				}

				matched = true;
			}
		}

		if (!matched){
			cerr << "Plate '" << exp.plate_name << "' (ID: " << exp.plate_id << ") not found in '" + plates_file + "'\n";
			fail = true;
		}
	}

	// Reverse check: whether all plates in the data file were expected. There may
	// be plates that we were not expecting to see.
	for (const PLPlate* plate : plates){
		bool matched = false;

		for (ExpectedPlatesEntry exp : expected_plates.getEntries()){
			if (exp.plate_id == plate->getId()) matched = true;
		}

		if (!matched){
			cerr << "Unexpected plate '" << plate->getName() << "' (ID: " << plate->getId() << ") encountered in '" + plates_file + "'\n";
			fail = true;
		}
	}

	if (fail) FAIL() << "Inconsistencies found in data file '" << plates_file << "'";

	// All good.
}

string PlateDataTest::_normalisePlateName(const string plate_name) {
	string res = plate_name;
	boost::algorithm::to_lower(res); // in-place!

	// We don't care about the difference between 'Elan Bank plate' and 'Elan Bank'
	boost::replace_all(res, "plate", ""); // in-place!

	boost::trim(res); // in-place

	return res;
}

void PlateDataTest::testLocation(double lat, double lon, unsigned int expected_plate_id, string expected_plate_name) {
	unsigned int foo_id;
	string foo_name;
	testLocation(lat, lon, expected_plate_id, expected_plate_name, foo_id, foo_name);
}

void PlateDataTest::testLocation(double lat, double lon, unsigned int expected_plate_id, string expected_plate_name, unsigned int& computed_plate_id, string& computed_plate_name) {
	string expected_plate_name_norm = PlateDataTest::_normalisePlateName(expected_plate_name);

	string kml_plate_name_norm;
	string gpml_plate_name_norm;
	int kml_plate_id;
	int gpml_plate_id;

	PLPlates* plp_kml = NULL;
	try {
		plp_kml = PLPlates::readFromFile("data/plates.kml");
		const PLPlate* plate_kml = plp_kml->findPlate(lat, lon);

		kml_plate_id = plate_kml->getId();
		kml_plate_name_norm = PlateDataTest::_normalisePlateName(plate_kml->getName());

		computed_plate_id = kml_plate_id;
		computed_plate_name = plate_kml->getName();
	} catch (...){}


	PLPlates* plp_gpml = NULL;
	try {
		plp_gpml = PLPlates::readFromFile("data/plates.gpml");
		const PLPlate* gpml_plate = plp_gpml->findPlate(lat, lon);

		gpml_plate_id = gpml_plate->getId();
		gpml_plate_name_norm = PlateDataTest::_normalisePlateName(gpml_plate->getName());

		computed_plate_id = gpml_plate_id;
		computed_plate_name = gpml_plate->getName();
	} catch (Exception& e){
		FAIL() << "Unexpected exception during point-on-plate computation: " << e.what();
	}


	if (plp_kml != NULL && plp_gpml != NULL){
		ASSERT_EQ(kml_plate_id, gpml_plate_id) << "Plate ID mismatch between plates.gpml and plates.kml: have the files been synchronised? Expected plate ID: " << expected_plate_id;
		ASSERT_EQ(kml_plate_name_norm, gpml_plate_name_norm) << "Plate name between plates.gpml and plates.kml: have the files been synchronised? Expected (normalised) plate name: " << expected_plate_name_norm;
	}

	if (plp_kml != NULL){
		ASSERT_EQ(expected_plate_id, kml_plate_id) << "Unexpected plate name in GPML file";
		ASSERT_EQ(expected_plate_name_norm, kml_plate_name_norm) << "Unexpected plate name in KML file";
	}


	if (plp_gpml != NULL){
		ASSERT_EQ(expected_plate_id, gpml_plate_id);
		ASSERT_EQ(expected_plate_name_norm, gpml_plate_name_norm);
	}

	ASSERT_TRUE(plp_kml != NULL || plp_gpml != NULL) << "No plate data (KML or GPML) found?";


	delete plp_kml;
	delete plp_gpml;
}


void PlateDataTest::ExpectedPlatesEntry::set(unsigned int col_index, const string value, const string filename, unsigned int lineno){
	// Column index 0: plate ID (uint)
	// Column index 1: plate name
	auto entries = _container->getEntries();

	if (col_index == 0) this->parseString(value, this->plate_id);
	if (col_index == 1)	this->plate_name = value;
}

size_t PlateDataTest::ExpectedPlatesEntry::numColumns() const {
	return 2;
}

