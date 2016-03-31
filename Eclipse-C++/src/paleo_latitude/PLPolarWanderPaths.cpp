/*
 * PLPolarWanderPaths.cpp
 *
 *  Created on: 30 Jun 2014
 *      Author: Sebastiaan J. van Schaik
 */

#include "PLPolarWanderPaths.h"

#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <sstream>

#include "exceptions/PLFileParseException.h"
#include "../util/Exception.h"

using namespace paleo_latitude;
using namespace std;


PLPolarWanderPaths::PLPolarWanderPaths() {}

PLPolarWanderPaths::~PLPolarWanderPaths() {
	delete _csvdata;
	_csvdata = NULL;
}

PLPolarWanderPaths* PLPolarWanderPaths::readFromFile(string filename) {
	PLPolarWanderPaths* res = new PLPolarWanderPaths();
	res->_readFromFile(filename);
	return res;
}

void PLPolarWanderPaths::_readFromFile(string filename){
	if (_csvdata != NULL) delete _csvdata;
	_csvdata = new CSVFileData<PWPEntry>();
	_csvdata->parseFile(filename);
}

const PLPolarWanderPaths::PWPEntry* PLPolarWanderPaths::getEntry(const PLPlate& plate, unsigned int age) const {
	return this->getEntry(plate.getId(), age);
}


const PLPolarWanderPaths::PWPEntry* PLPolarWanderPaths::getEntry(unsigned int plate_id, unsigned int age) const {
	for (const PWPEntry& entry : _csvdata->getEntries()){
		if (entry.age == age && entry.plate_id == plate_id) return &entry;
	}

	Exception ex;
	ex << "No apparent polar wander path known for plate ID " << plate_id << " and age " << age;

	throw ex;
}


const vector<PLPolarWanderPaths::PWPEntry>& PLPolarWanderPaths::getAllEntries() const {
	return _csvdata->getEntries();
}


void PLPolarWanderPaths::PWPEntry::set(unsigned int col_index, const string value, const string filename, unsigned int lineno){
	// Column index 0: plage ID (uint)
	// Column index 1: age (uint)
	// Column index 2: a95 (uint)
	// Column index 3: longitude (double)
	// Column index 4: latitude (double)

	auto entries = _container->getEntries();

	if (col_index == 0) this->parseString(value, this->plate_id);

	if (col_index == 1){
		this->parseString(value, this->age);
	}

	if (col_index == 2) this->parseString(value, this->a95);
	if (col_index == 3) this->parseString(value, this->latitude);
	if (col_index == 4) this->parseString(value, this->longitude);
}

size_t PLPolarWanderPaths::PWPEntry::numColumns() const {
	return 5;
}



