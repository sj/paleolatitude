/*
 * PLEulerPolesReconstructions.h
 *
 *  Created on: 3 Jul 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef PLEULERPOLESRECONSTRUCTIONS_H_
#define PLEULERPOLESRECONSTRUCTIONS_H_
#include <set>
#include "../util/CSVFileData.h"

namespace paleo_latitude {

class PLPlate;

/**
 * Data structure that processes and holds Euler pole reconstructions from a specific
 * model. It reads data from a single CSV file (e.g. 'euler-torsvik-2012.csv'), and
 * expects the following columns in this order:
 *  - plate ID (uint)
 *  - age (uint, in Myr)
 *  - latitude (double)
 *  - longitude (double)
 *  - rotation (double)
 *  - plate ID which the rotation is relative to (uint)
 *
 */
class PLEulerPolesReconstructions {
public:
	struct EPEntry : public CSVFileData<EPEntry>::Entry {
		EPEntry(CSVFileData<EPEntry>* parent, unsigned int line_no) : CSVFileData<EPEntry>::Entry(parent, line_no){}
		void set(unsigned int col_index, const string& value, const string& filename, unsigned int lineno) override;
		unsigned int numColumns() const override;

		static bool compareByAge(const EPEntry* a, const EPEntry* b);


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

	/**
	 * Returns the Euler poles for a given plate and age. Often, the result will be a single
	 * EPEntry, but in rare cases two entries will be returned. This happens at the cross-over
	 * point at which rotation is expressed relative to two plates (e.g. plate 102 at 320 Ma in Torsvik).
	 */
	vector<const EPEntry*> getEntries(unsigned int plate_id, unsigned int age) const;

	/**
	 * Returns all Euler poles for a given plate ID.
	 */
	vector<const EPEntry*> getEntries(unsigned int plate_id) const;

	/**
	 * Returns all plate IDs found in the Euler data
	 */
	set<unsigned int> getPlateIds() const;

	vector<const EPEntry*> getEntries(const PLPlate* plate, unsigned int age) const;

	const vector<EPEntry>& getAllEntries() const;

	static PLEulerPolesReconstructions* readFromFile(const string& filename);

private:
	PLEulerPolesReconstructions();
	void _readFromFile(const string& filename);

	CSVFileData<EPEntry>* _csvdata = NULL;
};

};

#endif /* PLEULERPOLESRECONSTRUCTIONS_H_ */
