/**
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

#include "paleo_latitude/PaleoLatitude.h"
#include "paleo_latitude/PLParameters.h"

#include <iostream>
#include <string>
#include <boost/program_options.hpp>

#include "util/Exception.h"
#include "debugging-macros.h"
#include "gtest-includes.h"
#include "util/Logger.h"

using namespace std;
using namespace paleo_latitude;

namespace bpo = boost::program_options;

void print_usage(bpo::options_description& cmdline_options_spec){
	cout << "Usage: paleolatitude [options]" << endl;
	cout << cmdline_options_spec << endl;
}

int main(int argc, char* argv[]) {
	// Parameters to Paleolatitude model implementation
	PLParameters* pl_params = new PLParameters();

	// Values of command-line parameters (through Boost Program Options library)
	bpo::variables_map cmdline_params_values;

	bpo::options_description cmdline_params_spec ("PaleoLatitude parameters");
	cmdline_params_spec.add_options()
		("help", "print usage and help information")
		("about", "print information about PaleoLatitude and exit")
		("version", "print model version number and exit")
		("site-lat", bpo::value<double>(&pl_params->site_latitude), "sets the site latitude")
		("site-lon", bpo::value<double>(&pl_params->site_longitude), "sets the site longitude")
		("age", bpo::value<double>(&pl_params->age), "set the desired age (in million years)")
		("min-age", bpo::value<double>(&pl_params->age_min), "set the lower bound on age (in million years)")
		("max-age", bpo::value<double>(&pl_params->age_max), "set the upper bound on age (in million years)")
		("age-error", bpo::value<double>(&pl_params->age_pm), "sets the error (+/-) around age (in million years)")
		("age-pm", bpo::value<double>(&pl_params->age_pm), "sets the error (+/-) around age (in million years, alias for age-error)")
		("input-apwp-csv", bpo::value<string>(&pl_params->input_apwp_csv)->default_value(pl_params->input_apwp_csv), "path to apparent polar wander paths specification of plates (in CSV format)")
		("input-euler-rotation-csv", bpo::value<string>(&pl_params->input_euler_rotation_csv)->default_value(pl_params->input_euler_rotation_csv), "path to specification of Euler rotation parameters of polar wander path (in CSV format)")
		("input-plates-file", bpo::value<string>(&pl_params->input_plates_file)->default_value(pl_params->input_plates_file), "path to specification of tectonic plates locations (GPML or KML format)")
		("csv-output-file", bpo::value<string>(), "enables detailed CSV output to specified file")
		("kml-output-file", bpo::value<string>(), "enables KML output of tectonic plates and site to specified file")
		("all-ages", "enable calculation of paleolatitude for all available ages (works best with --csv-output-file or --machine-readable)")
		("machine-readable", "enable machine readable output on standard output (includes CSV and KML)")
		("skip-about", "skips the header containing version and author information")
		("log-level", bpo::value<unsigned int>()->default_value(2), "sets the log level (0 = only errors, ..., 4 = debug. Default: 2)");

#ifndef __OPTIMIZE__
	cmdline_params_spec.add_options()
			("run-tests", "run unit tests (only available in non-optimised builds)")
			("gtest_catch_exceptions", bpo::value<string>(), "gtest unit test library flag")
			("gtest_filter", bpo::value<string>(), "gtest unit test library flag for running one or more specific test(s)");
#endif

	try {
		bpo::store(bpo::parse_command_line(argc, argv, cmdline_params_spec), cmdline_params_values);
		bpo::notify(cmdline_params_values);
	} catch (boost::program_options::error& ex){
		PaleoLatitude::printAbout();
		cout << endl;

		cerr << "Error: " << ex.what() << endl << endl;

		print_usage(cmdline_params_spec);
		exit(1);
	}

	// Disable unwanted loggers:
	//   level 0: only errors (disable 1 = warning, 2 = info, 3 = debug)
	//   level 1: warnings and errors (disable 2, 3)
	//   level 2: info, warnings, and errors (disable 3)
	//   level 3: debug, info, warnings, and errors
	const unsigned int log_level = cmdline_params_values["log-level"].as<unsigned int>();
	for (unsigned int disable_ll = log_level + 1; disable_ll <= 3; disable_ll++){
		if (disable_ll == 1) Logger::warning.disable();
		if (disable_ll == 2) Logger::info.disable();
		__IF_DEBUG(if(disable_ll == 3) Logger::debug.disable();)
	}

#ifndef __OPTIMIZE__
	// Run unit tests if requested (only available in debug builds - Release builds do not depend on Google Test)
	if (cmdline_params_values.count("run-tests")){
		::testing::InitGoogleTest(&argc, argv);
		const int test_result = RUN_ALL_TESTS();
		exit(test_result);
	}
#endif

	pl_params->all_ages = (cmdline_params_values.count("all-ages") > 0);

	if (cmdline_params_values.count("version") > 0){
		// Print version and exit
		cout << "PaleoLatitude " + PaleoLatitude::getVersion() << endl;
		exit(0);
	}

	if (cmdline_params_values.count("skip-about") == 0) PaleoLatitude::printAbout();

	if (cmdline_params_values.count("help") > 0){
		// Print usage and exit
		print_usage(cmdline_params_spec);
		exit(0);
	}

	if (cmdline_params_values.count("about") > 0) exit(0);

	// All logic related to the actual model parameters is located in PLParameters
	string validate_error_msg;
	if (!pl_params->validate(validate_error_msg)){
		cerr << validate_error_msg << endl << endl;
		print_usage(cmdline_params_spec);
		exit(1);
	}

	PaleoLatitude* pl = NULL;

	try {
		pl = new PaleoLatitude(pl_params);

		if (!pl->compute()){
			// Paleolatitude computation failed. Error message will have been printed to
			// the user.
			exit(1);
		}
	} catch (exception& ex){
		// Something went very wrong. Print message.
		cerr << "Unexpected error computing paleolatitude: " << ex.what() << endl;
		exit(1);
	}

	if (cmdline_params_values.count("machine-readable") == 0){
		// Output is intended for human user - print the basic paleolatitude result
		PaleoLatitude::PaleoLatitudeEntry res = pl->getPaleoLatitude();

		cout << "The paleolatitude of site (" << pl_params->site_latitude << "," << pl_params->site_longitude << ") ";
		if (res.age_years > 0 && PaleoLatitude::is_valid_latitude(res.palat)){
			// Specific age available in results data
			cout << "at age " << (res.age_years / 1000000.0) << " Myr is: ";
			cout << res.palat;

			if (PaleoLatitude::is_valid_latitude(res.palat_min) && PaleoLatitude::is_valid_latitude(res.palat_max)){
				cout << " (bounds: [" << res.palat_min << "," << res.palat_max << "])";
			} else{
				cout << " (bounds n/a)";
			}

			if ((res.age_years != res.age_years_lower_bound || res.age_years != res.age_years_upper_bound) && res.age_years_lower_bound >= 0 && res.age_years_upper_bound >= 0){
				// Result contains bound based on an age range with min/max other than requested age
				cout << " for age range [" << (res.age_years_lower_bound / 1000000.0) << "," << (res.age_years_upper_bound / 1000000.0) << "] Myr)";
			} // else: bounds are specifically for requested age

			cout << endl;
		} else {
			// No specific age requested (so: age bound or all ages) - just print the general bounds
			cout << "in age range [" << (res.age_years_lower_bound / 1000000.0) << "," << (res.age_years_upper_bound / 1000000.0) << "] Myr is: ";
			cout << "[" << res.palat_min << "," << res.palat_max << "]" << endl;
		}
	} else if (cmdline_params_values.count("machine-readable") > 0){
		// Output should be machine readable. Write to stdout:

		// #latitude:52.50
		// #longitude:-35.5
		// #plate_name:Eurasia
		// #plate_id:201
		// #CSV
		// .....
		//
		// #KML
		// .....

		cout << "#latitude:" << pl_params->site_latitude << endl;
		cout << "#longitude:" << pl_params->site_longitude << endl;

		cout << "#plate_name:" << pl->getPlate()->getName() << endl;
		cout << "#plate_id:" << pl->getPlate()->getId() << endl;

		cout << "#CSV" << endl;
		pl->writeCSV(cout);

		cout << endl;
		cout << "#KML" << endl;
		pl->writeKML(cout);
	}

	if (cmdline_params_values.count("csv-output-file") > 0){
		// Export CSV
		try{
			pl->writeCSV(cmdline_params_values["output-csv"].as<string>());
		} catch (exception& ex){
			cerr << "Error saving CSV file: " << ex.what() << endl;
			exit(1);
		}
	}

	if (cmdline_params_values.count("kml-output-file") > 0){
		// Export KML
		try{
			pl->writeKML(cmdline_params_values["output-kml"].as<string>());
		} catch (exception& ex){
			cerr << "Error saving KML file: " << ex.what() << endl;
			exit(1);
		}
	}

	delete pl;
	delete pl_params;

	return 0;
}
