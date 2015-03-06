/*
 * LogStream.h
 *
 *  Created on: 3 Jul 2014
 *      Author: bas
 */

#ifndef LOGSTREAM_H_
#define LOGSTREAM_H_

#include <iostream>
#include <string>

using namespace std;

namespace paleo_latitude {

class LogStream {
public:
	enum FlagTypes { FlagNoLabel };
	struct Flag {
		Flag(FlagTypes type_) : type(type_){}
		FlagTypes type;
	};

	LogStream(string label, ostream& target = cout);

	void enable();
	void disable();
	bool isEnabled() const;

	template <class SomeType> LogStream& operator<<(SomeType val){
		if (_enabled){
			if (!_skipLabel && _label != ""){
				_target << _label << ": ";
				_skipLabel = true;
			}
			_target << val;
		}
		return *this;
	}

	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
	typedef CoutType& (*StandardEndLine)(CoutType&);
	LogStream& operator<<(StandardEndLine manip);

	LogStream& operator<<(Flag someFlag);

	static Flag noLabel;

private:
	ostream& _target;
	bool _enabled = true;
	bool _skipLabel = false;
	string _label = "";
};

};

#endif /* LOGSTREAM_H_ */
