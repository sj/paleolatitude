//============================================================================
// Name        : main.cpp (part of PaleoLatitude)
// Author      : Sebastiaan J. van Schaik
// Copyright   : Sebastiaan J. van Schaik
// Licence     : GNU Lesser GPL 3.0
// Description :
//============================================================================
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


/**
 * Runs unit tests (through Google Test framework) if one of the command-line parameters requests it. Terminates
 * after that with the exit code specified by the tests framework. Note that tests are only available in
 * Debug builds.
 */
void runUnitTestsIfRequested(int argc, char* argv[]){
	bool run_tests = false;

	for (int i = 0; i < argc; i++){
		const string arg = argv[i];
		if (arg == "--run-unit-tests" || arg == "--run-tests" || arg == "run-tests" || arg == "runtests"){
			run_tests = true;
			break;
		}
	}

	if (run_tests){
#ifndef __OPTIMIZE__
		// Call Google Test
		::testing::InitGoogleTest(&argc, argv);
		const int test_result = RUN_ALL_TESTS();
		exit(test_result);
#else
		cerr << "Error: unit tests are only available in Debug builds." << endl;
		exit(1);
#endif
	} // else: don't run tests
}

int main(int argc, char* argv[]) {
	// First and foremost: run unit tests when requested
	runUnitTestsIfRequested(argc, argv);

	// Parameters to Paleolatitude model implementation
	PLParameters* pl_params = new PLParameters();

	// Values of command-line parameters (through Boost Program Options library)
	bpo::variables_map cmdline_params_values;

	// TODO: check description of 'input-euler-rotation-csv' with Douwe

	bpo::options_description cmdline_params_spec ("PaleoLatitude parameters");
	cmdline_params_spec.add_options()
		("help", "print usage and help information")
		("about", "print information about PaleoLatitude and exit")
		("site-lat", bpo::value<double>(&pl_params->site_latitude), "sets the site latitude")
		("site-lon", bpo::value<double>(&pl_params->site_longitude), "sets the site longitude")
		("age", bpo::value<double>(&pl_params->age), "set the desired age (in million years)")
		("min-age", bpo::value<double>(&pl_params->age_min), "set the lower bound on age (in million years)")
		("max-age", bpo::value<double>(&pl_params->age_max), "set the upper bound on age (in million years)")
		("age-error", bpo::value<double>(&pl_params->age_pm), "sets the error (+/-) around age (in million years)")
		("age-pm", bpo::value<double>(&pl_params->age_pm), "sets the error (+/-) around age (in million years, alias for age-error)")
		("input-apwp-csv", bpo::value<string>(&pl_params->input_apwp_csv)->default_value(pl_params->input_apwp_csv), "path to apparent polar wander paths specification of plate 701 (in CSV format)")
		("input-euler-rotation-csv", bpo::value<string>(&pl_params->input_euler_rotation_csv)->default_value(pl_params->input_euler_rotation_csv), "path to specification of Euler rotation parameters of polar wander path (in CSV format)")
		("input-plates-file", bpo::value<string>(&pl_params->input_plates_file)->default_value(pl_params->input_plates_file), "path to specification of tectonic plates locations (GPML or KML format)")
		("csv-output-file", bpo::value<string>(), "enables detailed CSV output to specified file")
		("kml-output-file", bpo::value<string>(), "enables KML output of tectonic plates and site to specified file")
		("all-ages", "enable calculation of paleolatitude for all available ages (works best with --csv-output-file or --machine-readable)")
		("machine-readable", "enable machine readable output on standard output (includes CSV and KML)")
		("skip-about", "skips the header containing version and author information")
		("log-level", bpo::value<unsigned int>()->default_value(2), "sets the log level (0 = only errors, ..., 4 = debug. Default: 2)");

	try {
		bpo::store(bpo::parse_command_line(argc, argv, cmdline_params_spec), cmdline_params_values);
		bpo::notify(cmdline_params_values);
	} catch (boost::exception_detail::clone_impl<boost::exception_detail::error_info_injector<boost::program_options::unknown_option> >& ex){
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

	pl_params->all_ages = (cmdline_params_values.count("all-ages") > 0);

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
		if (cmdline_params_values.count("all-ages") == 0){
			// Specific age specified
			PaleoLatitude::PaleoLatitudeEntry res = pl->getPaleoLatitude();
			cout << "The paleolatitude of site (" << pl_params->site_latitude << "," << pl_params->site_longitude << ") in age range [" << pl_params->getMinAge() << "," << pl_params->getMaxAge() << "]: ";
			cout << res.palat << " (bounds: [" << res.palat_min << "," << res.palat_max << "])" << endl;
		} else {
			// All ages requested - just print the general bounds
			pair<double,double> bounds = pl->getPaleoLatitudeBounds();
			cout << "The paleolatitude bounds of site (" << pl_params->site_latitude << "," << pl_params->site_longitude << ") for all ages are: ";
			cout << "[" << bounds.first << "," << bounds.second << "]" << endl;
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
