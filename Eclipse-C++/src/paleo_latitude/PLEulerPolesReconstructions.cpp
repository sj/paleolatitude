/*
 * PLEulerPolesReconstructions.cpp
 *
 *  Created on: 3 Jul 2014
 *      Author: Sebastiaan J. van Schaik
 */

#include "PLEulerPolesReconstructions.h"

#include "PLPlate.h"

using namespace std;
using namespace paleo_latitude;


PLEulerPolesReconstructions::PLEulerPolesReconstructions() {
}

PLEulerPolesReconstructions::~PLEulerPolesReconstructions(){
	delete _csvdata;
	_csvdata = NULL;
}

PLEulerPolesReconstructions* PLEulerPolesReconstructions::readFromFile(const string& filename) {
	PLEulerPolesReconstructions* res = new PLEulerPolesReconstructions();
	res->_readFromFile(filename);
	return res;
}

void PLEulerPolesReconstructions::_readFromFile(const string& filename) {
	if (_csvdata != NULL) delete _csvdata;
	_csvdata = new CSVFileData<EPEntry>();
	_csvdata->parseFile(filename);
}


vector<unsigned int> PLEulerPolesReconstructions::getRelevantAges(const PLPlate* plate, unsigned int min, unsigned int max){
	vector<unsigned int> res;

	int left_outside_age = -1;
	int right_outside_age = -1;

	for (const EPEntry& entry : _csvdata->getEntries()){
		if (entry.plate_id != plate->getId()) continue;


		if (entry.age >= min && entry.age <= max){
			// CSV entry falls in between the min/max: this age should be included
			// in the ages.
			res.push_back(entry.age);
		}

		if (entry.age <= min){
			// Entry is on window border, or just outside window. Let's
			// see whether we need it.
			if (left_outside_age == -1 || (unsigned int) left_outside_age < entry.age){
				// The previously found left-outside entry is less relevant than
				// this entry
				left_outside_age = entry.age;
			}
		}

		if (entry.age >= max){
			// Entry is on/outside window border on right
			if (right_outside_age == -1 || (unsigned int) right_outside_age > entry.age){
				right_outside_age = entry.age;
			}
		}
	}

	if (left_outside_age >= 0 && (unsigned int) left_outside_age < min){
		// Include left-outside
		res.push_back(left_outside_age);
	} // else: no left-outside, or left outside is on window border (and therefore already included)


	if (right_outside_age >= 0 && (unsigned int) right_outside_age > max){
		// Include left-outside
		res.push_back(right_outside_age);
	} // else: no right-outside, or is on window border (and therefore already included)


	return res;
}


void PLEulerPolesReconstructions::EPEntry::set(unsigned int col_index, const string& value, const string& filename, unsigned int lineno){
	// Column index 0: plate id (uint)
	// Column index 1: age (uint)
	// Column index 2: latitude (double)
	// Column index 3: longitude (double)
	// Column index 4: rotation (double)
	// Column index 5: plate id (uint) to which the rotation is relative

	if (col_index == 0) this->parseString(value, this->plate_id, filename, lineno);
	if (col_index == 1) this->parseString(value, this->age, filename, lineno);
	if (col_index == 2) this->parseString(value, this->latitude, filename, lineno);
	if (col_index == 3) this->parseString(value, this->longitude, filename, lineno);
	if (col_index == 4) this->parseString(value, this->rotation, filename, lineno);
	if (col_index == 5) this->parseString(value, this->rotation_rel_to_plate_id, filename, lineno);
}

unsigned int PLEulerPolesReconstructions::EPEntry::numColumns() const {
	return 6;
}

const PLEulerPolesReconstructions::EPEntry& PLEulerPolesReconstructions::getEntry(const PLPlate* plate, unsigned int age) const {
	for (const EPEntry& entry : _csvdata->getEntries()){
		if (entry.age == age && entry.plate_id == plate->getId()) return entry;
	}
	Exception ex;
	ex << "No entry for age=" << age << " and plate_id=" << plate->getId() << " found in Euler pole table?";
	throw ex;
}


const vector<PLEulerPolesReconstructions::EPEntry>& PLEulerPolesReconstructions::getEntries() const {
	return _csvdata->getEntries();
}
