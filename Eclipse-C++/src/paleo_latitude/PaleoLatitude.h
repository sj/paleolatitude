/*
 * PaleoLatitude.h
 *
 *  Created on: 4 Jun 2014
 *      Author: Sebastiaan J. van Schaik
 *
 *
 * Paleolatitude model Copyright (c) Sebastiaan J. van Schaik, All rights
 * reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License (version 3.0) as
 * published by the Free Software Foundation.

 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef PALEOLATITUDE_H_
#define PALEOLATITUDE_H_
#include <iomanip>
#include <string>
#include <boost/numeric/ublas/matrix.hpp>
#include <cstdio>
#include <tuple>
#include <iostream>

#include "PLPlate.h"

#define PALEOLATITUDE_VERSION 0.98

using namespace std;
namespace bnu = boost::numeric::ublas;

namespace paleo_latitude {

class PLPolarWanderPaths;
class PLParameters;
class PLPlates;
class PLEulerPolesReconstructions;

/**
 * The PaleoLatitude class contains the model logic and ties all data (e.g. plates, euler table)
 * together. It takes a PLParameters pointer that describes the input data.
 */
class PaleoLatitude {

public:
	struct PaleoLatitudeEntry {
		PaleoLatitudeEntry() : age_years(0), palat_min(0), palat(0), palat_max(0) {}
		PaleoLatitudeEntry(unsigned long age_years_, double palat_min_, double palat_, double palat_max_) :
			age_years(age_years_), palat_min(palat_min_), palat(palat_), palat_max(palat_max_){}

		string to_string();
		static PaleoLatitudeEntry interpolate(const PaleoLatitudeEntry& other_younger, const PaleoLatitudeEntry& other_older, unsigned long age_years);

		double getAgeInMIY() const;
		unsigned long age_years;
		double palat_min, palat, palat_max;
		bool is_interpolated = false;
	};

	PaleoLatitude();
	PaleoLatitude(PLParameters* params);
	PaleoLatitude(const PaleoLatitude& other) = delete;

	virtual ~PaleoLatitude();

	/**
	 * Writes the paleolatitude data to a CSV file
	 */
	void writeCSV(const string& filename);

	/**
	 * Writes the paleolatitude data to an output_stream
	 */
	void writeCSV(ostream& output_stream);

	/**
	 * Writes KML data of tectonic plates to a file
	 */
	void writeKML(const string& filename);

	/**
	 * Writes KML data of tectonic plates to an output_stream
	 */
	void writeKML(ostream& output_stream);

	/**
	 * Returns the main paleolatitude result (age, paleolatitude, lower bound, upper bound).
	 * Cannot be called when the paleolatitude of all ages is requested.
	 */
	PaleoLatitudeEntry getPaleoLatitude() const;

	/**
	 * Returns the lower and upper bound of all paleolatitude results. Mostly useful when all
	 * ages have been requested.
	 */
	pair<double,double> getPaleoLatitudeBounds() const;

	/**
	 * Quick-access method to set the parameters of this PaleoLatitude
	 */
	PLParameters* set();

	/**
	 * Returns the plate that is used for paleolatitude calculations (determined based on the
	 * input parameters)
	 */
	const PLPlate* getPlate() const;

	/**
	 * Computes the paleolatitude based on the input parameters and input data
	 */
	bool compute();

	/**
	 * Prints information about this implementation to stdout
	 */
	static void printAbout();

private:
	PLPolarWanderPaths* _pwp = NULL;
	PLParameters* _params = NULL;
	PLPlates* _plates = NULL;
	PLEulerPolesReconstructions* _euler = NULL;
	const PLPlate* _plate = NULL;

	vector<PaleoLatitudeEntry> _result;

	PaleoLatitudeEntry _calculatePaleolatitudeRangeForAge(const Coordinate& site, const PLPlate* plate, unsigned int age);

	template<class T> static string _ppMatrix(const bnu::matrix<T>& matrix);
	template<class T> static string _ppVector(const bnu::vector<T>& vector);

	static double _deg2rad(const double& deg);
	static double _rad2deg(const double& deg);

	void _requireResult() const;
};


template<class T>
string PaleoLatitude::_ppMatrix(const bnu::matrix<T>& matrix) {
	stringstream res;
	res.precision(5);
	res << fixed;

	res << "Matrix (dimensions: " << matrix.size1() << "x" << matrix.size2() << ")" << endl;
	for (unsigned int row = 0; row < matrix.size1(); row++){
		for (unsigned int col = 0; col < matrix.size2(); col++){
			const double val = matrix(row,col);
			if (col != 0) res << ", ";
			if (val >= 0) res << " ";
			res << val;
		}
		res << endl;
	}
	return res.str();
}


template<class T> string PaleoLatitude::_ppVector(const bnu::vector<T>& vector) {
	stringstream res;
	res.precision(5);
	res << fixed;
	res << "[";
	for (unsigned int i = 0; i < vector.size(); i++){
		if (i != 0) res << ",";
		res << vector[i];
	}
	res << "]";
	return res.str();
};

};
#endif /* PALEOLATITUDE_H_ */
