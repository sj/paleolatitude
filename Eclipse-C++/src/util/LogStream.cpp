/*
 * LogStream.cpp
 *
 *  Created on: 3 Jul 2014
 *      Author: bas
 */

#include "LogStream.h"

using namespace paleo_latitude;

LogStream::LogStream(string label, ostream& target) : _target(target), _label(label) {}


void LogStream::enable() {
	_enabled = true;
}

void LogStream::disable() {
	_enabled = false;
}

bool LogStream::isEnabled() const {
	return _enabled;
}

LogStream& LogStream::operator<<(Flag someFlag){
	if (someFlag.type == FlagTypes::FlagNoLabel){
		_skipLabel = true;
	}
	return *this;
}

LogStream& LogStream::operator<<(StandardEndLine manip){
	_skipLabel = false;
	if (_enabled) _target << manip;
	return *this;
}

LogStream::Flag LogStream::noLabel = Flag(FlagTypes::FlagNoLabel);
