/*
 * Logger.cpp
 *
 *  Created on: 29 Jun 2014
 *      Author: Sebastiaan J. van Schaik
 */

#include "Logger.h"
#include <iostream>
#include "LogStream.h"

using namespace paleo_latitude;

void Logger::logError(string str_error){
	error << str_error << endl;
}

void Logger::logInfo(string str_info){
	info << str_info << endl;
}


void Logger::logWarn(string str_warn){
	warning << str_warn << endl;
}

void Logger::disableAll(){
	error.disable();
	info.disable();
	warning.disable();
#ifdef __DEBUG__
	debug.disable();
#endif
}


void Logger::enableAll(){
	error.enable();
	info.enable();
	warning.enable();
#ifdef __DEBUG__
	debug.enable();
#endif
}


LogStream Logger::info = LogStream("INFO");
LogStream Logger::warning = LogStream("WARNING");
LogStream Logger::error = LogStream("ERROR", cerr);

#ifdef __DEBUG__
LogStream Logger::debug = LogStream("DEBUG");
#endif
