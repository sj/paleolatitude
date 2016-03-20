/*
 * PLEulerPolesReconstructions.h
 *
 *  Created on: 3 Jul 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef PLEULERPOLESRECONSTRUCTIONS_H_
#define PLEULERPOLESRECONSTRUCTIONS_H_

#include "../util/CSVFileData.h"

namespace paleo_latitude {

class PLPlate;

class PLEulerPolesReconstructions {
public:
	struct EPEntry : public CSVFileData<EPEntry>::Entry {
		EPEntry(const CSVFileData<EPEntry>& parent) : CSVFileData<EPEntry>::Entry(parent){}
		void set(unsigned int col_index, const string& value, const string& filename, unsigned int lineno) override;
		unsigned int numColumns() const override;

		unsigned int plate_id = 0;
		unsigned int age = 0;
		unsigned int rotation_rel_to_plate_id = 0;
		double latitude = 0;
		double longitude = 0;
		double rotation = 0;
	};

	virtual ~PLEulerPolesReconstructions();

	/**
	 * Returns all ages relevant to a paleolatitude query from min to max.
	 */
	vector<unsigned int> getRelevantAges(const PLPlate* plate, unsigned int min, unsigned int max);

	const EPEntry& getEntry(const PLPlate* plate, unsigned int age) const;
	const vector<EPEntry>& getEntries() const;

	static PLEulerPolesReconstructions* readFromFile(const string& filename);

private:
	PLEulerPolesReconstructions();
	void _readFromFile(const string& filename);

	CSVFileData<EPEntry>* _csvdata = NULL;
};

};

#endif /* PLEULERPOLESRECONSTRUCTIONS_H_ */
