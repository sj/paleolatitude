/*
 * PLPolarWanderPaths.cpp
 *
 *  Created on: 30 Jun 2014
 *      Author: bas
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


PLPolarWanderPaths::PWPEntry PLPolarWanderPaths::getEntry(unsigned int age) {
	for (const PWPEntry& entry : _csvdata->getEntries()){
		if (entry.age == age) return entry;
	}
	throw Exception("No apparent polar wander path known for specified age");
}


const vector<PLPolarWanderPaths::PWPEntry>& PLPolarWanderPaths::getEntries() const {
	return _csvdata->getEntries();
}


void PLPolarWanderPaths::PWPEntry::set(unsigned int col_index, const string& value, const string& filename, unsigned int lineno){
	// Column index 0: age (uint)
	// Column index 1: a95 (uint)
	// Column index 2: longitude (double)
	// Column index 3: latitude (double)

	auto entries = _container.getEntries();

	if (col_index == 0){
		this->parseString(value, this->age, filename, lineno);

		if (entries.size() > 0 && entries.back().age > age){
			// Last entry has larger age than this entry - expecting them in increasing order
			throw PLFileParseException("Expecting data entries in '" + filename + "' to have strictly increasing age");
		}
	}
	if (col_index == 1) this->parseString(value, this->a95, filename, lineno);
	if (col_index == 2) this->parseString(value, this->latitude, filename, lineno);
	if (col_index == 3) this->parseString(value, this->longitude, filename, lineno);
}

unsigned int PLPolarWanderPaths::PWPEntry::numColumns() const {
	return 4;
}



