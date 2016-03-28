/*
 * PaleoLatitudeParameters.h
 *
 *  Created on: 4 Jun 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef PALEOLATITUDEPARAMETERS_H_
#define PALEOLATITUDEPARAMETERS_H_

#include <string>

using namespace std;
namespace paleo_latitude {

class PLParameters {

public:
	bool validate() const;
	bool validate(string& error_msg) const;

	double getMinAge() const;
	double getMaxAge() const;

	unsigned long getMinAgeInYears() const;
	unsigned long getMaxAgeInYears() const;

	bool hasAge() const;
	unsigned long getAgeInYears() const;

	string input_apwp_csv = "data/apwp-torsvik-2012-vandervoo-2015.csv";
	string input_euler_rotation_csv = "data/euler-torsvik-2012-vandervoo-2015.csv";
	string input_plates_file = "data/plates.gpml";

	// Initialise to default (invalid) values
	double site_latitude = -9999;
	double site_longitude = -9999;
	double age = -9999;
	double age_max = -9999;
	double age_min = -9999;
	double age_pm = -9999;

	bool all_ages = false;
};

};

#endif /* PALEOLATITUDEPARAMETERS_H_ */
