/*
 * PLParameters.cpp
 *
 *  Created on: 4 Jun 2014
 *      Author: Sebastiaan J. van Schaik
 */

#include "PLParameters.h"

#include <iostream>
#include <cmath>
#include "../util/Exception.h"
using namespace paleo_latitude;

bool PLParameters::validate() const {
	string ignore_string;
	return validate(ignore_string);
}

bool PLParameters::validate(string& err_msg) const {
	const string err_msg_one_of_three =  "Error in input parameters: expecting exactly one of (1) both age and age-pm, (2) min-age, and max-age, or (3) all-ages";

	if (age < 0 && age_min < 0 && age_max < 0 && !all_ages){
		// None of the age specifications given. Need one.
		err_msg = err_msg_one_of_three;
		return false;
	}

	if (age_max >= 0 || age_min >= 0){
		// At least one of age_min,age_max provided

		if (age_pm >= 0 || all_ages){
			// Cannot provide arguments for age plus-minus and/or all-ages as well
			err_msg = err_msg_one_of_three;
			return false;
		}

		// Expecting both age-min, age-max
		if (age_max < 0 || age_min < 0){
			err_msg = "Error in input parameters: only one of age-min or age-max specified, please specify either both or neither";
			return false;
		}

		// If age is specified, then it has to be within min max
		if (age >= 0 && (age > age_max || age < age_min)){
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

	// TODO: we should be able to compute without range!
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
	if (!this->hasAge()) throw Exception("no age specified - cannot compute age in Myr");
	return static_cast<unsigned long>(round(age * 1000000));
}

bool PLParameters::hasAge() const {
	return age > -0.001;
}
