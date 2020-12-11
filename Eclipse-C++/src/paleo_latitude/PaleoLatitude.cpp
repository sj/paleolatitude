/*
 * PaleoLatitude.cpp
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

#include "../debugging-macros.h"
#include "PaleoLatitude.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/assignment.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include <iostream>
#include <cmath>
#include <cstdlib>

#include "PLPlate.h"
#include "PLParameters.h"
#include "PLPlates.h"
#include "PLPolarWanderPaths.h"
#include "../util/Logger.h"
#include "PLEulerPolesReconstructions.h"

using namespace paleo_latitude;
using namespace std;

namespace bnu = boost::numeric::ublas;

PaleoLatitude::PaleoLatitude() {

}


PaleoLatitude::~PaleoLatitude() {
	delete _pwp;
	delete _plates;
	delete _euler;

	_pwp = NULL;
	_plates = NULL;
	_euler = NULL;
}


PaleoLatitude::PaleoLatitude(PLParameters* params) : _params(params) {
	_pwp = PLPolarWanderPaths::readFromFile(params->input_apwp_csv);
	_plates = PLPlates::readFromFile(params->input_plates_file);
	_euler = PLEulerPolesReconstructions::readFromFile(params->input_euler_rotation_csv);
}

PLParameters* PaleoLatitude::set() {
	return _params;
}


bool PaleoLatitude::compute(){
	string validate_err;
	if (!_params->validate(validate_err)){
		throw Exception(validate_err);
	}

	// Determine plate
	const Coordinate site(_params->site_latitude, _params->site_longitude);
	_plate = _plates->findPlate(site);

	Logger::info << "Site " << site.to_string() << " (lat,lon) is located on plate '" << _plate->getName() << "' (id: " << _plate->getId() << ")" << endl;

	if (_plate->getId() == 1001){
		// Unconstrained plate - can't do anything with that
		Logger::error << "The provided site is located on an unconstrained plate - cannot compute paleolatitude" << endl;
		return false;
	}

	const double age_myr = _params->age;
	const double age_max_myr = _params->getMaxAge();
	const double age_min_myr = _params->getMinAge();
	const long age_min_years = _params->getMinAgeInYears();
	const long age_max_years = _params->getMaxAgeInYears();

	// Read the relevant ages from the Euler rotations table
	vector<unsigned int> compute_ages;
	if (_params->all_ages){
		compute_ages = _euler->getRelevantAges(_plate, 0, 99999);
	} else if (age_min_myr >= 0 && age_max_myr >= 0) {
		compute_ages = _euler->getRelevantAges(_plate, age_min_myr, age_max_myr);
	} else {
		compute_ages = _euler->getRelevantAges(_plate, age_myr, age_myr);
	}

	if (compute_ages.size() == 0){
		Logger::error << "Insufficient data available to compute paleolatitude for site (" << _params->site_latitude << "," << _params->site_longitude << ") on plate " << _plate->getName() << " (id: " << _plate->getId() << ") for the requested age(s). Maybe try computing for all ages?";
		return false;
	}

	Logger::info << "Computing lower and upper bound of paleolatitude for the following ages: ";
	for (unsigned int rel_age_myr : compute_ages) Logger::info << rel_age_myr << " ";
	Logger::info << endl;

	// Paleolatitudes for a series of ages:
	// age, paleolat_min, paleolat, paleolat_max
	_result.clear();
	_result.reserve(compute_ages.size() * 2);

	// Compute values for relevant ages, interpolated values will be added later
	for (unsigned int i = 0; i < compute_ages.size(); i++){
		const unsigned int curr_age_myr = compute_ages[i];

		const vector<PaleoLatitudeEntry> palats = _calculatePaleolatitudeRangeForAge(site, _plate, curr_age_myr);
		_result.insert(std::end(_result), std::begin(palats), std::end(palats));
	}

	// Sort the results (from more recent to longer ago)
	sort(_result.begin(), _result.end(), PaleoLatitude::PaleoLatitudeEntry::compareByAge);

	// Perform interpolation (if needed)
	unsigned int size_before_interpolation = _result.size();

	for (unsigned int i = 0; i < _result.size() - 1; i++){ // note the deliberate size() - 1 !
		PaleoLatitudeEntry& curr_entry = _result[i];
		PaleoLatitudeEntry& next_entry = _result[i+1]; // possible because loop ranges to size() - 2

		const unsigned int curr_age_myr = curr_entry.getAgeInMYR();
		const unsigned int next_age_myr = next_entry.getAgeInMYR();

		if (age_min_myr > curr_age_myr && age_min_myr < next_age_myr){
			// age_min lies between the current and the next age - interpolate
			PaleoLatitudeEntry interpolated = PaleoLatitudeEntry::interpolate(curr_entry, next_entry, age_min_years);
			_result.push_back(interpolated);
		}

		if (_params->hasAge()){
			// Specific age requested - make sure we actually provide an answer
			// for that age (we might have to interpolate)
			const double age_years = _params->getAgeInYears();

			if (age_myr > curr_age_myr && age_myr < next_age_myr){
				// requested age in between current and next age - interpolate
				PaleoLatitudeEntry interpolated = PaleoLatitudeEntry::interpolate(curr_entry, next_entry, age_years);
				_result.push_back(interpolated);
			}
		} // else: no specific age requested

		if (age_max_myr > curr_age_myr && age_max_myr < next_age_myr){
			// age_max in between current and next age - interpolate
			PaleoLatitudeEntry interpolated = PaleoLatitudeEntry::interpolate(curr_entry, next_entry, age_max_years);
			_result.push_back(interpolated);
		}
	}


	if (size_before_interpolation != _result.size()){
		// Interpolated data added to end of result vector - sort again.
		sort(_result.begin(), _result.end(), PaleoLatitude::PaleoLatitudeEntry::compareByAge);
	}

	for (PaleoLatitudeEntry entry : _result){
		Logger::info << entry.to_string() << endl;
	}

	return true;
}



/**
 * Step 3
 */
const vector<PaleoLatitude::PaleoLatitudeEntry> PaleoLatitude::_calculatePaleolatitudeRangeForAge(const Coordinate& site, const PLPlate* plate, unsigned int age_myr) const {
	__IF_DEBUG(Logger::debug << "Calculating paleolatitude for (lat=" << site.latitude << ",lon=" << site.longitude << ") for age=" << age_myr << endl);

	// Get Euler pole and reference pole. For some ages, this will yield multiple (up to two)
	// Euler poles (relative to different plates)
	const vector<const PLEulerPolesReconstructions::EPEntry*> euler_entries = _euler->getEntries(plate, age_myr);

	vector<PaleoLatitude::PaleoLatitudeEntry> res;
	for (const PLEulerPolesReconstructions::EPEntry* euler_entry : euler_entries){
		const PLPolarWanderPaths::PWPEntry* pwp_entry = _pwp->getEntry(euler_entry->rotation_rel_to_plate_id, age_myr);
		res.push_back(_calculatePaleolatitudeRange(site, plate, age_myr, euler_entry, pwp_entry));
	}

	return res;
}

const PaleoLatitude::PaleoLatitudeEntry PaleoLatitude::_calculatePaleolatitudeRange(const Coordinate& site, const PLPlate* plate, unsigned int age_myr, const PLEulerPolesReconstructions::EPEntry* euler_entry, const PLPolarWanderPaths::PWPEntry* pwp_entry) const {
	const double lambda_s = site.latitude;
	const double lambda_s_rad = _deg2rad(lambda_s);
	const double phi_s  = site.longitude;
	const double phi_s_rad = _deg2rad(phi_s);

	const double lambda_e = euler_entry->latitude;
	const double lambda_e_rad = _deg2rad(lambda_e);
	const double phi_e = euler_entry->longitude;
	const double phi_e_rad = _deg2rad(phi_e);
	const double omega = euler_entry->rotation;
	const double omega_rad = _deg2rad(omega);


	// Get latitude and longitude of reference pole from
	// relevant entry from apparent polar wander paths

	const double lambda_p = pwp_entry->latitude;
	const double lambda_p_rad = _deg2rad(lambda_p);
	const double phi_p  = pwp_entry->longitude;
	const double phi_p_rad = _deg2rad(phi_p);
	const double a95 = pwp_entry->a95;
	const bool compute_bounds = (a95 > 0.0000001);

	const double theta_e = 90 - lambda_e;		// colatitude of Euler pole
	const double theta_e_rad = _deg2rad(theta_e);
	const double theta_p = 90 - lambda_p;		// colatitude of reference pole
	const double theta_p_rad = _deg2rad(theta_p);

	__IF_DEBUG(Logger::debug << "λ_s = " << lambda_s << " (" << lambda_s_rad << "), φ_s = " << phi_s << " (" << phi_s_rad << "), age = " << age_myr << " (Myr)" << endl;)
	__IF_DEBUG(Logger::debug << "λ_E = " << lambda_e << " (" << lambda_e_rad << "), φ_E = " << phi_e << " (" << phi_e_rad << "), Ω = " << omega << " (" << omega_rad << ")" << endl;)
	__IF_DEBUG(Logger::debug << "λ_p = " << lambda_p << " (" << lambda_p_rad << "), φ_p = " << phi_p << " (" << phi_p_rad << "), A95 = " << a95 << (compute_bounds ? "" : " (n/a)") <<  endl;)
	__IF_DEBUG(Logger::debug << "θ_E = " << theta_e << " (" << theta_e_rad << "), θ_p = " << theta_p << " (" << theta_p_rad << ")" << endl;)

	// Unit vectors
	const bnu::unit_vector<unsigned int> vec_x_unit(3,0); //  [1, 0, 0]
	const bnu::unit_vector<unsigned int> vec_y_unit(3,1); //  [0, 1, 0]
	const bnu::unit_vector<unsigned int> vec_z_unit(3,2); //  [0, 0, 1]


	bnu::vector<double> vec_theta_e (3);
	vec_theta_e <<=	cos(phi_e_rad) * cos(theta_e_rad),
					sin(phi_e_rad) * cos(theta_e_rad),
					-sin(theta_e_rad);

	// Consistency check:
	const double vec_theta_e_sqrt = sqrt(pow(vec_theta_e[0],2.0) + pow(vec_theta_e[1],2.0) + pow(vec_theta_e[2],2.0));
	__IF_DEBUG(Logger::debug << "θ_e[] = [" << vec_theta_e[0] << "," << vec_theta_e[1] << "," << vec_theta_e[2] << "]  (square root of squared sums: " << vec_theta_e_sqrt << ")" << endl;)
	if (!Util::double_eq(vec_theta_e_sqrt, 1)){
		Exception ex;
		ex << "Consistency check failed: square root of squared sums of θ_E[] vector elements does not equal 1, but " << vec_theta_e_sqrt << "?";
		throw ex;
	}


	bnu::vector<double> vec_phi_e (3);
	vec_phi_e <<=	-sin(phi_e_rad),
					cos(phi_e_rad),
					0;

	// Consistency check:
	const double vec_phi_e_sqrt = sqrt(pow(vec_phi_e[0],2.0) + pow(vec_phi_e[1],2.0) + pow(vec_phi_e[2],2.0));
	__IF_DEBUG(Logger::debug << "φ_e[] = [" << vec_phi_e[0] << "," << vec_phi_e[1] << "," << vec_phi_e[2] << "]  (square root of squared sums: " << vec_theta_e_sqrt << ")" << endl;)
	if (!Util::double_eq(vec_phi_e_sqrt, 1)){
		Exception ex;
		ex << "Consistency check failed: square root of squared sums of r_E[] vector elements does not equal 1, but " << vec_phi_e_sqrt << "?";
		throw ex;
	}

	bnu::vector<double> vec_r_e (3);	// rotation axis
	vec_r_e <<=	cos(phi_e_rad) * sin(theta_e_rad),
				sin(phi_e_rad) * sin(theta_e_rad),
				cos(theta_e_rad);

	// Consistency check:
	const double vec_r_e_sqrt = sqrt(pow(vec_r_e[0],2.0) + pow(vec_r_e[1],2.0) + pow(vec_r_e[2],2.0));
	__IF_DEBUG(Logger::debug << "r_e[] = [" << vec_r_e[0] << "," << vec_r_e[1] << "," << vec_r_e[2] << "]  (square root of squared sums: " << vec_theta_e_sqrt << ")" << endl;)
	if (!Util::double_eq(vec_r_e_sqrt, 1)){
		Exception ex;
		ex << "Consistency check failed: square root of squared sums of r_E[] vector elements does not equal 1, but " << vec_r_e_sqrt << "?";
		throw ex;
	}

	// Transformation matrix:
	bnu::matrix<double> L(3, 3);
	L(0,0) = inner_prod(vec_theta_e, vec_x_unit);
	L(1,0) = inner_prod(vec_theta_e, vec_y_unit);
	L(2,0) = inner_prod(vec_theta_e, vec_z_unit);

	L(0,1) = inner_prod(vec_phi_e, vec_x_unit);
	L(1,1) = inner_prod(vec_phi_e, vec_y_unit);
	L(2,1) = inner_prod(vec_phi_e, vec_z_unit);

	L(0,2) = inner_prod(vec_r_e, vec_x_unit);
	L(1,2) = inner_prod(vec_r_e, vec_y_unit);
	L(2,2) = inner_prod(vec_r_e, vec_z_unit);

	__IF_DEBUG(Logger::debug << "Transformation matrix L: " << _ppMatrix(L) << endl;)

	const bnu::matrix<double> L_trans = trans(L);
	__IF_DEBUG(Logger::debug << "L_transposed: " << _ppMatrix(L_trans) << endl;)

	// Reference pole to Cartesian coordinates:
	bnu::vector<double> vec_xyz_p(3);
	vec_xyz_p <<=	cos(phi_p_rad) * sin(theta_p_rad),
					sin(phi_p_rad) * sin(theta_p_rad),
					cos(theta_p_rad);

	__IF_DEBUG(Logger::debug << "xyz[] = " << _ppVector(vec_xyz_p) << "    (reference pole -> cartesian coordinates)"<< endl;)


	bnu::matrix<double> rot_matrix(3,3);
	rot_matrix <<=	cos(-omega_rad),-sin(-omega_rad),	0,
					sin(-omega_rad),cos(-omega_rad),	0,
					0,			0,				1;


	__IF_DEBUG(Logger::debug << "Rotation matrix R: " << _ppMatrix(rot_matrix) << endl;)

	const bnu::matrix<double> L_x_rot = prod(L, rot_matrix);
	__IF_DEBUG(Logger::debug << "L*R: " << _ppMatrix(L_x_rot) << endl;)

	// Multiply transposed L (Lt) with cartesian coordinates of reference pole (xyz)
	const bnu::vector<double> Lt_x_xyz = prod(L_trans, vec_xyz_p);
	__IF_DEBUG(Logger::debug << "L_trans * xyz: " << _ppVector(Lt_x_xyz) << "     (transposed L * cartesian coordinates reference pole)" << endl;)

	const bnu::vector<double> vec_xyz_p_rot = prod(L_x_rot, Lt_x_xyz);
	__IF_DEBUG(Logger::debug << "xyz_p_rot = " << _ppVector(vec_xyz_p_rot) << "     (= result of Euler pole rotation)" << endl;)

	const double& x_p_rot = vec_xyz_p_rot(0);
	const double& y_p_rot = vec_xyz_p_rot(1);
	const double& z_p_rot = vec_xyz_p_rot(2);

	// if x_p_rot < 0: 						phi_p_rot = atan(y_p_rot / x_p_rot) + pi;
	// if x_p_rot >= 0 and y_p_rot >= 0: 	phi_p_rot = atan(y_p_rot / x_p_rot)
	// if x_p_rot >= 0 and y_p_rot <= 0:	phi_p_rot = atan(y_p_rot / x_p_rot) + 2 * pi;
	double phi_p_rot_rad = atan(y_p_rot / x_p_rot);
	if (x_p_rot < 0) phi_p_rot_rad += M_PI;
	if (x_p_rot >= 0 and y_p_rot <= 0) phi_p_rot_rad += 2 * M_PI;

	const double theta_p_rot_rad = acos(z_p_rot);

	const double lambda_p_rot_rad = 0.5 * M_PI - theta_p_rot_rad;

	__IF_DEBUG(Logger::debug << "φ_p_rot = " << phi_p_rot_rad << ", θ_p_rot = " << theta_p_rot_rad << ", λ_p_rot = " << lambda_p_rot_rad << " (radians)" << endl;)


	// Paleolatitude
	const double lambda_numerator = sin(lambda_p_rot_rad) * sin(lambda_s_rad) +
			cos(lambda_p_rot_rad) * cos(lambda_s_rad) * cos(phi_p_rot_rad - phi_s_rad);
	const double lambda_denominator = sqrt(1 - pow(lambda_numerator, 2.0));

	const double lambda_rad = atan(lambda_numerator / lambda_denominator);
	const double lambda = _rad2deg(lambda_rad);

	// Uncertainty in paleolatitude
	double lambda_min, lambda_max;

	if (compute_bounds){
		// A95 data available for computation of error bounds
		const double delta_i = a95 * 2.0 / (1 + 3 * pow(cos(0.5*M_PI - lambda_rad), 2.0));
		const double delta_i_rad = _deg2rad(delta_i);
		__IF_DEBUG(Logger::debug << "Λ = " << lambda_rad << " (radians), Δ_I = " << delta_i << " degrees, Δ_I = " << delta_i_rad << " radians" << endl;)

		// Upper and lower bounds for paleolatitude

		// Inclination of geomagnetic field (I)
		const double field_incl_rad = atan((2 * lambda_numerator) / lambda_denominator);
		__IF_DEBUG(Logger::debug << "I = " << field_incl_rad << " (inclination of geomagnetic field in radians)" << endl;)

		const double lambda_min_rad = atan(0.5 * tan(field_incl_rad - delta_i_rad));
		const double lambda_max_rad = atan(0.5 * tan(field_incl_rad + delta_i_rad));


		lambda_min = _rad2deg(lambda_min_rad);
		lambda_max = _rad2deg(lambda_max_rad);


		__IF_DEBUG(Logger::debug << "Λ_min = " << lambda_min_rad << ", Λ = " << lambda_rad << ", Λ_max = " << lambda_max_rad << " (radians)" << endl;)
		__IF_DEBUG(Logger::debug << "Λ_min = " << lambda_min << ", Λ = " << lambda << ", Λ_max = " << lambda_max << " (degrees)" << endl;)


		// Check whether either lambda_min or lambda_max is out of bounds, indicating that the lower or upper
		// bound moved over a pole.
		if (lambda_max < lambda || lambda_min > lambda){
			// Either:
			// Upper bound of paleolatitude moved up over the north pole, then down on the other side. The
			// reported max value then looks like -85 (= 85 degrees on the other side)
			// Or:
			// Lower bound moved over the south pole, then up the other side. Resulting min is then
			// 85, which should be -85.

			Logger::info << "Applying correction for bounds over south pole: [" << lambda_min << "," << lambda_max << "] becomes ";
			if (lambda < 0){
				// Things went south on the south pole (ba dum tssh)
				lambda_max = max(-abs(lambda_min), -abs(lambda_max));
				lambda_min = -90;
			} else {
				// Things went wrong on north pole
				lambda_min = min(abs(lambda_min), abs(lambda_max));
				lambda_max = 90;
			}

			Logger::info << "[" << lambda_min << "," << lambda_max << "]" << endl;
		}
	} else {
		// No uncertainty bounds computable due to lack of A95 info
		__IF_DEBUG(Logger::debug << "not computing error bounds on paleolatitude due to lack of data" << endl;)

		lambda_min = -99999;
		lambda_max = -99999;
	}

	const unsigned long age_years = age_myr * 1000000;
	return PaleoLatitudeEntry(age_years, age_years, age_years, lambda_min, lambda, lambda_max, euler_entry->rotation_rel_to_plate_id);
}

double PaleoLatitude::_deg2rad(const double& deg){
	return (deg * M_PI) / 180.0;
}

double PaleoLatitude::_rad2deg(const double& rad){
	return rad * (180.0 / M_PI);
}

string PaleoLatitude::getVersion(){
	return PALEOLATITUDE_VERSION;
}

void PaleoLatitude::printAbout() {
	cout << "This is PaleoLatitude version " << PALEOLATITUDE_VERSION << " (http://www.paleolatitude.org)" << endl;
	cout << "Implementation by Sebastiaan J. van Schaik (sjvs@traiectum.net)" << endl;
	cout << "Source code licensed under the GNU Lesser GPL (LGPL) version 3.0"<< endl << endl;

	cout << "Please cite:" << endl;
	cout << "  Douwe J.J. van Hinsbergen, Lennart V. de Groot, Sebastiaan J. van Schaik," << endl;
	cout << "  Appy Sluijs, Peter K. Bijl, Wim Spakman, Cor G. Langereis, Henk Brinkhuis:" << endl;
	cout << "  A Paleolatitude Calculator for Paleoclimate Studies" << endl;
	cout << "  In: PLoS ONE, 2015 (http://doi.org/10.1371/journal.pone.0126946)." << endl << endl;
}

string PaleoLatitude::PaleoLatitudeEntry::to_string() {
	stringstream res;
	double age_myr = (age_years / 1000000.0);
	res << "PaleoLatitude for age " << age_myr << " (Myr): ";

	if (is_valid_latitude(palat_min)){
		res << "Λ_min = " << palat_min << ", ";
	} else {
		res << "Λ_min = n/a, ";
	}

	res << "Λ = " << palat << ", ";

	if (is_valid_latitude(palat_max)){
		res << "Λ_max = " << palat_max;
	} else {
		res << "Λ_max = n/a";
	}

	if (is_interpolated){
		res << " (interpolated)";
	} else {
		res << " (using polar wander path of plate " << computed_using_plate_id << ")";
	}
	return res.str();
}

PaleoLatitude::PaleoLatitudeEntry PaleoLatitude::PaleoLatitudeEntry::interpolate(
		const PaleoLatitudeEntry& other_younger,
		const PaleoLatitudeEntry& other_older, unsigned long age_years) {

	if (other_younger.computed_using_plate_id != other_older.computed_using_plate_id){
		Exception e;
		e << "Cannot interpolate for age=" << age_years << " between two paleolatitude results that were computed using two different reference plates";
		throw e;
	}

	const double delta_palat_min = other_older.palat_min - other_younger.palat_min;
	const double delta_palat = other_older.palat - other_younger.palat;
	const double delta_palat_max = other_older.palat_max - other_younger.palat_max;
	const double delta_age = other_older.age_years - other_younger.age_years;
	const double rel_age = age_years - other_younger.age_years;

	const double palat_min = other_younger.palat_min + (delta_palat_min / delta_age) * rel_age;
	const double palat = other_younger.palat + (delta_palat / delta_age) * rel_age;
	const double palat_max = other_younger.palat_max + (delta_palat_max / delta_age) * rel_age;

	PaleoLatitudeEntry res = PaleoLatitudeEntry(age_years, age_years, age_years, palat_min, palat, palat_max, other_younger.computed_using_plate_id);
	res.is_interpolated = true;
	return res;
}

void PaleoLatitude::_requireResult() const {
	if (_result.size() == 0) throw Exception("PaleoLatitude not computed - call compute() first");
}

const vector<PaleoLatitude::PaleoLatitudeEntry> PaleoLatitude::getRelevantPaleolatitudeEntries() const {
	return _result;
}

/**
 * Returns the result of the paleolatitude computation for a specific site and specific age bounds. This method cannot be
 * used when the paleolatitude of an age range (or of all ages) is computed.
 *
 * @see getPaleolatitudeBounds
 */
PaleoLatitude::PaleoLatitudeEntry PaleoLatitude::getPaleoLatitude() const {
	_requireResult();

	__IF_DEBUG(Logger::debug << "Processing " << _result.size() << " results for requested time period" << endl;)

	// The computation might have yielded multiple results, even if only a single
	// age was requested: if that exact age is not available, then the results will
	// have been interpolated. In that case, the bounds of the whole set of data
	// points need to be inspected: the min and max need to be returned.
	PaleoLatitudeEntry aggr;
	aggr.age_years = 99999; // placeholder to trigger later initialisation

	for (PaleoLatitudeEntry entry : _result){
		if (aggr.age_years == 99999){
			aggr = entry; // initialise with useful values
			aggr.age_years = -99999;
			aggr.palat = 0;
		}

		if (is_valid_latitude(entry.palat_min) && (entry.palat_min < aggr.palat_min || !is_valid_latitude(aggr.palat_min))) aggr.palat_min = entry.palat_min;
		if (is_valid_latitude(entry.palat_max) && (entry.palat_max > aggr.palat_max || !is_valid_latitude(aggr.palat_max))) aggr.palat_max = entry.palat_max;
		if (is_valid_latitude(entry.palat) && (entry.palat < aggr.palat_min || !is_valid_latitude(aggr.palat_min))) aggr.palat_min = entry.palat;
		if (is_valid_latitude(entry.palat) && (entry.palat > aggr.palat_max || !is_valid_latitude(aggr.palat_max))) aggr.palat_max = entry.palat;

		if (entry.age_years_lower_bound < aggr.age_years_lower_bound) aggr.age_years_lower_bound = entry.age_years_lower_bound;
		if (entry.age_years_upper_bound > aggr.age_years_upper_bound) aggr.age_years_upper_bound = entry.age_years_upper_bound;

		if (_params->hasAge() && entry.age_years  == _params->getAgeInYears()){
			aggr.palat = entry.palat;
			aggr.age_years = entry.age_years;
		}
	}

	return aggr;
}

void PaleoLatitude::writeCSV(const string& filename) {
	ofstream out(filename);
	writeCSV(out);
	out.close();
}

void PaleoLatitude::writeKML(const string& filename) {
	ofstream out(filename);
	writeKML(out);
	out.close();
}

void PaleoLatitude::writeCSV(ostream& output_stream) {
	_requireResult();
	output_stream << "age;latitude;lower bound;upper bound;interpolated;relative_to" << endl;
	output_stream.setf(ios::fixed, ios::floatfield);

	for (PaleoLatitudeEntry entry : _result){
		output_stream.precision(2);
		output_stream << entry.getAgeInMYR() << ";";

		output_stream.precision(5);
		output_stream << entry.palat << ";";

		if (is_valid_latitude(entry.palat_min)) output_stream << entry.palat_min;
		output_stream << ";";

		if (is_valid_latitude(entry.palat_max)) output_stream << entry.palat_max;
		output_stream << ";";

		output_stream << (entry.is_interpolated ? "1" : "0") << ";";

		if (entry.computed_using_plate_id > 0){
			const string plate_name = _plates->getPlateName(entry.computed_using_plate_id);
			if (plate_name == ""){
				output_stream << "Plate " << entry.computed_using_plate_id;
			} else {
				output_stream << plate_name << " (" << entry.computed_using_plate_id << ")";
			}
		}

		output_stream << endl;
	}
}

void PaleoLatitude::writeKML(ostream& output_stream) {
	_requireResult();

	output_stream << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>" << endl
			<< "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << endl
			<< "<Document>" << endl
			<< " <name>paleolatitude.org</name>" << endl
			<< " <Style id=\"paleolatitude_site_default\">" << endl
			<< "  <IconStyle>" << endl
			<< "   <scale>0.75</scale>" << endl
			<< "   <Icon><href>http://maps.google.com/mapfiles/kml/shapes/target.png</href></Icon>" << endl
			<< "  </IconStyle>" << endl
			<< " </Style>" << endl
			<< " <StyleMap id=\"paleolatitude_site\">" << endl
			<< "  <Pair>" << endl
			<< "   <key>normal</key><styleUrl>#paleolatitude_site_default</styleUrl>" << endl
			<< "  </Pair>" << endl
			<< "  <Pair>" << endl
			<< "   <key>highlight</key><styleUrl>#paleolatitude_site_default</styleUrl>" << endl
			<< "  </Pair>" << endl
			<< " </StyleMap>" << endl;

	// Add Placemark for site location
	output_stream << "<Placemark>" << endl
			<< " <name>Site location</name>" << endl
			<< " <open>1</open>" << endl
			<< " <styleUrl>#paleolatitude_site</styleUrl>" << endl
			<< " <Point><coordinates>" << _params->site_longitude << "," << _params->site_latitude << "</coordinates></Point>" << endl
			<< "</Placemark>" << endl;


	// Add Placemarks with Polygons
	for (const PLPlate* plate : _plates->getPlates()){
		if (_plate == plate){
			// Make site plate red
			plate->writeKMLPlacemark(output_stream, "ff0000ff");
		} else {
			// Other plates are default colour (blue)
			plate->writeKMLPlacemark(output_stream);
		}
	}


	output_stream << "</Document>" << endl
			<< "</kml>" << endl;
}

double PaleoLatitude::PaleoLatitudeEntry::getAgeInMYR() const {
	return (age_years / 1000000.0);
}

bool PaleoLatitude::PaleoLatitudeEntry::compareByAge(const PaleoLatitudeEntry& a, const PaleoLatitudeEntry& b) {
	if (a.age_years == b.age_years){
		// Entries that were computed relative to Africa come first. This makes sure that
		// the youngest paleolatitude entries (which are all computed relative to Africa)
		// come first, before relative movement to other plates is considered (for older ages).
		if (a.computed_using_plate_id > 0 && b.computed_using_plate_id > 0){
			if (a.computed_using_plate_id == 701) return true;
			if (b.computed_using_plate_id == 701) return false;
		}
	}
	return (a.age_years < b.age_years);
}

const PLPlate* PaleoLatitude::getPlate() const {
	_requireResult();
	return _plate;
}
