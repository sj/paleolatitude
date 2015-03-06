/*
 * PLPlates.h
 *
 *  Created on: 3 Jul 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef PLPLATES_H_
#define PLPLATES_H_

#include <string>
#include "PLPlate.h"
#include "../util/Exception.h"
using namespace std;

namespace paleo_latitude {


class PLPlates {
public:
	virtual ~PLPlates();

	const PLPlate* findPlate(const Coordinate& site) const;
	const PLPlate* findPlate(double lat, double lon) const;

	const vector<const PLPlate*> getPlates() const;

	static PLPlates* readFromFile(const string& filename);

private:
	PLPlates();
	PLPlates(const PLPlates& other) = delete; // keep things easy: no copy constructor

	void _readPlatesFromKML(const string& kmlfilename);
	void _readPlatesFromGPML(const string& gpmlfilename);

	vector<const PLPlate*> _plates;
};

};

#endif /* PLPLATES_H_ */
