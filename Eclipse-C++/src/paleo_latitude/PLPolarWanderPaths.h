/*
 * PLPolarWanderPaths.h
 *
 *  Created on: 30 Jun 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef PLPOLARWANDERPATHS_H_
#define PLPOLARWANDERPATHS_H_
#include <vector>
#include "PLPlate.h"
#include "../util/Logger.h"
#include "../util/Util.h"
#include "../util/CSVFileData.h"
#include <string>
using namespace std;

namespace paleo_latitude {

/**
 * Class to deal with the apparent polar wander paths of the African plate
 */
class PLPolarWanderPaths {
public:
	struct PWPEntry : public CSVFileData<PWPEntry>::Entry {
		PWPEntry(const CSVFileData<PWPEntry>& parent, unsigned int line_no) : CSVFileData<PWPEntry>::Entry(parent, line_no){}
		void set(unsigned int col_index, const string& value, const string& filename, unsigned int lineno) override;
		unsigned int numColumns() const override;

		unsigned int plate_id = 0;
		unsigned int age = 0;
		double a95 = 0;
		double latitude = 0;
		double longitude = 0;
	};

	PLPolarWanderPaths(const PLPolarWanderPaths&) = delete;
	~PLPolarWanderPaths();

	const PWPEntry getEntry(unsigned int plate_id, unsigned int age) const;
	const PWPEntry getEntry(const PLPlate& plate, unsigned int age) const;
	const vector<PWPEntry>& getAllEntries() const;

	static PLPolarWanderPaths* readFromFile(string filename);

private:
	PLPolarWanderPaths();
	void _readFromFile(string filename);


	template<class T> bool _parseCsvField(unsigned int line_no, const string& field, T& result, const string& warn_msg){
		if (!Util::string_to_something(field, result)){
			if (warn_msg != ""){
				stringstream ss_warn;
				ss_warn << "Ignoring line " << line_no << " of 'filename': invalid age (expecting integer)";
				Logger::logWarn(ss_warn.str());
			}
			return false;
		}
		return true;
	}

	CSVFileData<PWPEntry>* _csvdata = NULL;
};

};

#endif /* PLPOLARWANDERPATHS_H_ */
