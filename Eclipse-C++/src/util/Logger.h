/*
 * Logger.h
 *
 *  Created on: 29 Jun 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "../debugging-macros.h"
#include <string>
#include <iostream>

#include "LogStream.h"
using namespace std;

using namespace std;

namespace paleo_latitude {

class Logger {
public:
	static void disableAll();
	static void enableAll();

	static void logError(string error);
	static void logInfo(string info);
	static void logWarn(string warn);

	static LogStream info;
	static LogStream warning;
	static LogStream error;

#ifdef __DEBUG__
	static LogStream debug;
#endif
};

}

#endif /* LOGGER_H_ */
