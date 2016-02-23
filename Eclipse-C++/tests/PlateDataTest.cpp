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

TEST_F(PlateDataTest, TestAmsterdam){
	PlateDataTest::testLocation(52.366667, 4.9, 301, "Eurasia");
}

TEST_F(PlateDataTest, TestNewYork){
	PlateDataTest::testLocation(40.7127, -74.0059, 101, "North America");
}

TEST_F(PlateDataTest, TestCapetown){
	PlateDataTest::testLocation(-33.925278, 18.423889, 701, "South Africa");
}

TEST_F(PlateDataTest, TestAntananarivoMadagascar){
	PlateDataTest::testLocation(-18.933333, 47.516667, 702, "Madagascar");
}

TEST_F(PlateDataTest, TestNewDelhi){
	PlateDataTest::testLocation(28.613889, 77.208889, 501, "India");
}

TEST_F(PlateDataTest, TestSydney){
	PlateDataTest::testLocation(-33.859972, 151.209444, 801, "Australia");
}


TEST_F(PlateDataTest, TestNuuk){
	PlateDataTest::testLocation(64.175, -51.738889, 102, "Greenland");
}

TEST_F(PlateDataTest, TestRussia){
	PlateDataTest::testLocation(60, 30, 301, "Eurasia");
}

TEST_F(PlateDataTest, TestSiberia){
	PlateDataTest::testLocation(60, 90, 401, "Somewhere in Siberia");
}

TEST_F(PlateDataTest, TestElanBank){
	// Elan Bank is plate-within-plate
	PlateDataTest::testLocation(-60, +60, 514, "Elan Bank");
}


TEST_F(PlateDataTest, TestHonolulu){
	PlateDataTest::testLocation(21.3, -157.816667, 901, "Pacific");
}

TEST_F(PlateDataTest, TestPalikirMicronesia){
	PlateDataTest::testLocation(6.917222, 158.158889, 901, "Pacific");
}

TEST_F(PlateDataTest, TestMarakesh){
	// Marakesh, Morocco. On recently introduced plate "Western Meseta"
	PlateDataTest::testLocation(31.63, -8.008889, 3547, "Western Meseta");
}

TEST_F(PlateDataTest, TestMascara){
	// Mascara, Northern Algeria. On recently introduced plate "Eastern Meseta south of Tell Belt"
	PlateDataTest::testLocation(35.383333, 0.15, 3546, "Eastern Meseta south of Tell Belt");
}

TEST_F(PlateDataTest, TestLongyearbyen){
	// Longyearbyen is on the Eurasian plate, but very close to both the Greenland plate,
	// and the North American Plate
	PlateDataTest::testLocation(78.22, 15.65, 301, "Eurasia");
}

TEST_F(PlateDataTest, TestNumberOfPlates){
	// This test reads the number of plates to expect from plates.num, and verifies whether
	// that's the number of plates found in the GPML/KML files

	int expected_plate_count = -1;
	int num_kml_plates = -1;
	int num_gpml_plates = -1;

	ifstream num_plates_file("data/plates.num");
	ASSERT_TRUE(num_plates_file.is_open()) << "File 'plates.num' not readable - cannot verify plate count";

	try {
		string currline;
		while (std::getline(num_plates_file, currline)){
			boost::trim(currline);

			if (currline.empty()) continue; // ignore empty lines

			if (boost::starts_with(currline, "#")) continue; // ignore comment

			// If it's not a comment, it should be a single number on a line on its own
			istringstream(currline) >> expected_plate_count;

			ASSERT_GT(expected_plate_count, 0) << "Invalid plate count provided in 'plates.num'";
			break;
		}
		num_plates_file.close();
	} catch (...){} // Exception not relevant

	PLPlates* plp_kml = NULL;
	try {
		plp_kml = PLPlates::readFromFile("data/plates.kml");
		num_kml_plates = plp_kml->getPlates().size();
	} catch (...){} // No worries if file not readable - just use GPML

	PLPlates* plp_gpml = NULL;
	try {
		plp_gpml = PLPlates::readFromFile("data/plates.gpml");
		num_gpml_plates = plp_gpml->getPlates().size();
	} catch (...){} // No worries if file not readable - just use KML

	if (plp_kml != NULL && plp_gpml != NULL){
		ASSERT_TRUE(num_kml_plates == num_gpml_plates) << "KML and GPML files do not agree on plate counts";
	}

	ASSERT_GT(expected_plate_count, 0) << "No plate count provided in 'plates.num'";
	if (plp_kml) ASSERT_EQ(expected_plate_count, num_kml_plates);
	if (plp_gpml) ASSERT_EQ(expected_plate_count, num_gpml_plates);

	delete plp_kml;
	delete plp_gpml;
}


TEST_F(PlateDataTest, TestNorthPole){
	// Arbitrary location in a small bit of North American Plate, enclosed by the Eurasian and
	// Greenland plates
	PlateDataTest::testLocation(80.952410, -4.012213, 101, "North America");

	// Just east of Greenland in the sea, very close to the North American plate
	PlateDataTest::testLocation(82.536376, -10.279457, 102, "Greenland");

	// Piece of Eurasian plate enclosed by North American and Greenland plates
	PlateDataTest::testLocation(80.215851, -2.679118, 301, "Eurasia");

	// North-east Siberian plate, close to some other plates around the north pole, and very close
	// to the 180 degree meridian
	PlateDataTest::testLocation(76.082751, 178.79066, 409, "Northeast Siberia");
}

TEST_F(PlateDataTest, TestSouthPole){
	PlateDataTest::testLocation(-89, 1, 802, "East Antarctica");
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
	} catch (...){}


	PLPlates* plp_gpml = NULL;
	try {
		plp_gpml = PLPlates::readFromFile("data/plates.gpml");
		const PLPlate* gpml_plate = plp_gpml->findPlate(lat, lon);

		gpml_plate_id = gpml_plate->getId();
		gpml_plate_name_norm = PlateDataTest::_normalisePlateName(gpml_plate->getName());
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
