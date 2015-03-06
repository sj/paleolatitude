/*
 * PLParameters.cpp
 *
 *  Created on: 4 Jun 2014
 *      Author: bas
 */

#include "PLParameters.h"

#include <iostream>
#include <cmath>
using namespace paleo_latitude;

bool PLParameters::validate() const {
	string ignore_string;
	return validate(ignore_string);
}

bool PLParameters::validate(string& err_msg) const {
	if (age < 0 && !all_ages){
		err_msg = "Error in input parameters: age not specified.";
		return false;
	}

	const string err_msg_one_of_three =  "Error in input parameters: expecting exactly one of (1) both age and age-pm, (2) age, min-age, and max-age, or (3) all-ages";

	if (age_max >= 0 || age_min >= 0){
		if (age_pm >= 0 || all_ages){
			err_msg = err_msg_one_of_three;
			return false;
		}

		// Expecting age-min, age-max, and age
		if (age_max < 0 || age_min < 0){
			err_msg = "Error in input parameters: only one of age-min or age-max specified, please specify either both or neither";
			return false;
		}
		if (age > age_max || age < age_min || age < 0){
			err_msg = "Error in input parameters: age not specified or not within bounds of [age_min, age_max]";
			return false;
		}
	}

	if (age_pm >= 0){
		if (age_min >= 0 || age_max >= 0 || all_ages){
			err_msg = err_msg_one_of_three;
			return false;
		}

		if (age < 0){
			err_msg = "Error in input parameters: age-pm specified, but age missing or invalid";
			return false;
		}
	}

	if (age >= 0 && age_pm < 0 && age_min < 0 && age_max < 0){
		// No age bounds specified
		err_msg = "Error in input parameters: no age bounds specified (through age-min/age-max, or age-pm)";
		return false;
	}

	if (all_ages && (age >= 0 || age_max >= 0 || age_min >= 0 || age_pm >= 0)){
		err_msg = err_msg_one_of_three;
		return false;
	}

	if (site_latitude > 90.001 || site_latitude < -90.001){
		err_msg = "Invalid latitude specified. Expecting a latitude in the range [-90, 90]";
		return false;
	}

	if (site_longitude > 180.001 || site_longitude < -180.001){
		err_msg = "Invalid site longitude specified. Expecting a longitude in the range [-180,180]";
		return false;
	}


	return true;
}



double PLParameters::getMaxAge() const {
	return age_max >= 0 ? age_max : (age + age_pm);
}


double PLParameters::getMinAge() const {
	return age_min >= 0 ? age_min : (age - age_pm);
}

unsigned long paleo_latitude::PLParameters::getMinAgeInYears() const {
	return static_cast<unsigned long>(round(getMinAge() * 1000000));
}

unsigned long paleo_latitude::PLParameters::getMaxAgeInYears() const {
	return static_cast<unsigned long>(round(getMaxAge() * 1000000));
}

unsigned long paleo_latitude::PLParameters::getAgeInYears() const {
	return static_cast<unsigned long>(round(age * 1000000));
}
