/*
 * PLPlate.h
 *
 *  Created on: 29 Jun 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef PLPLATE_H_
#define PLPLATE_H_

#include <string>
#include <vector>
#include <iostream>
using namespace std;

namespace paleo_latitude {

struct Coordinate {
	Coordinate(double latitude_, double longitude_) : latitude(latitude_), longitude(longitude_){}

	string to_string() const;

	const double latitude;
	const double longitude;
};

class PLPlate {
public:
	PLPlate(unsigned int plate_id, string plate_name, vector<Coordinate>* coordinates);
	PLPlate() = delete;

	virtual ~PLPlate();

	string getName() const;
	unsigned int getId() const;
	const vector<Coordinate>* getCoordinates() const;

	bool contains(const Coordinate& some_point) const;
	void writeKMLPlacemark(ostream& output_stream, string linecolor = "440000ff") const;

	string _ppCoordinates() const;

private:
	const unsigned int _id;
	const string _name;
	vector<Coordinate>* _polygon_coordinates;
};

};


#endif /* PLPLATE_H_ */
