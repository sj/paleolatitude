/*
 * CSVFileData.h
 *
 *  Created on: 3 Jul 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef CSVFILEDATA_H_
#define CSVFILEDATA_H_

#include "Exception.h"

#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "Util.h"

namespace paleo_latitude {

template<class EntryType>
class CSVFileData {
public:
	struct Entry {
		virtual void set(unsigned int column, const string& value, const string& filename, unsigned int lineno) = 0;
		virtual unsigned int numColumns() const = 0;

		template<class T> T parseString(const string& input, T& output, const string& filename, unsigned int lineno){
			bool res = Util::string_to_something(input, output);
			if (!res){
				CSVFileDataParseException ex;
				ex << "Parse exception on line " << lineno << " of '" << filename << "': unexpected string '" << input << "'";
				throw ex;
			}
			return output;
		}

	protected:
		Entry(const CSVFileData<EntryType>& parent) : _container(parent){}
		const CSVFileData<EntryType>& _container;

	private:
		Entry() = delete;
	};


	class CSVFileDataParseException : public Exception {
	public:
		CSVFileDataParseException(): Exception(){}
		CSVFileDataParseException(string msg) : Exception(msg){}
	};



	CSVFileData() {}

	const vector<EntryType>& getEntries() const {
		return _data;
	}

	void parseFile(const string& filename){
		ifstream csvfile (filename);

		if (!csvfile.good()){
			CSVFileDataParseException ex;
			ex << "File '" << filename << "' does not exist or is not readable";
			throw ex;
		}

		string line;

		unsigned int lines_parsed = 0;
		unsigned int line_no = 0;

		while (getline(csvfile, line)){
			line_no++;
			vector<string> values;

			EntryType csv_entry(*this);

			boost::split(values, line, boost::is_any_of(";,"));

			if (values.size() != csv_entry.numColumns()){
				if (lines_parsed > 0 || line_no >= 3){
					// Expecting n fields, got something else
					CSVFileDataParseException ex;
					ex << "Parse error on line " << line_no << " of '" << filename << "': expecting " << csv_entry.numColumns() << " values, got " << values.size();
					throw ex;
				} // else: header (at most two lines), just ignore

				continue;
			}

			try {
				for (unsigned int i = 0; i < values.size(); i++){
					csv_entry.set(i, values[i], filename, line_no);
				}

				_data.push_back(csv_entry);
				lines_parsed++;
			} catch (CSVFileDataParseException& pex){
				// Ignore parse exceptions in headers (ie., when no lines have been parsed yet)
				if (lines_parsed > 0 || line_no >= 3) throw;

				continue;
			}
		}

		if (lines_parsed == 0){
			throw CSVFileDataParseException("Could not parse information from '" + filename + "': no valid comma-separated lines found");
		}
	}

private:
	CSVFileData(const CSVFileData<EntryType>& other) = delete;

	vector<EntryType> _data;
};

};

#endif /* CSVFILEDATA_H_ */
