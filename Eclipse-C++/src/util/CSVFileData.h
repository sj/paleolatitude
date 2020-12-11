/*
 * CSVFileData.h
 *
 *  Created on: 3 Jul 2014
 *      Author: Sebastiaan J. van Schaik
 */

#ifndef CSVFILEDATA_H_
#define CSVFILEDATA_H_

#include "Exception.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "Util.h"

namespace paleo_latitude {

/**
 * Simple utility class for reading CSV files. Does not do support quoting or escaping.
 */
template<class EntryType>
class CSVFileData {
public:
	struct Entry {
	public:
		virtual void set(unsigned int column, const string value, const string filename, unsigned int lineno) = 0;
		virtual size_t numColumns() const = 0;

		template<class T> T parseString(const string& input, T& output) const {
			bool res = Util::string_to_something(input, output);

			if (!res){
				CSVFileDataParseException ex;
				ex << "Parse exception on line " << _line_no << " of '" << _container->getFilename() << "': unexpected string '" << input << "'";
				throw ex;
			}
			return output;
		}

		unsigned int getLineNo() const {
			return _line_no;
		}

	protected:
		Entry(CSVFileData<EntryType>* parent, unsigned int line_no) : _container(parent), _line_no(line_no){}
		CSVFileData<EntryType>* _container;
		unsigned int _line_no;

	private:
		Entry() = delete;
	};


	struct StringEntry : public Entry {
	public:
		virtual void set(unsigned int column, const string value, const string filename, unsigned int lineno){
			_values.resize(this->numColumns());
			_values[column] = value;
		}

		/**
		 * Tests whether the provided column is empty (i.e., whether the string in that column is empty)
		 */
		bool empty(unsigned int col_index) const {
			return _values.at(col_index).empty();
		}

		template<class T> T get_as(unsigned int col_index, T& output) const {
			return this->parseString(_values.at(col_index), output);
		}

		int get_int(unsigned int col_index) const {
			int res;
			return get_as(col_index, res);
		}

		double get_double(unsigned int col_index) const {
			double res;
			return get_as(col_index, res);
		}

		unsigned int get_uint(unsigned int col_index) const {
			unsigned int res;
			return get_as(col_index, res);
		}

		string get(unsigned int col_index) const {
			return _values[col_index];
		}

	protected:
		StringEntry(CSVFileData<EntryType>* parent, unsigned int line_no)  : Entry(parent, line_no){}

	private:
		vector<string> _values;
	};


	class CSVFileDataParseException : public Exception {
	public:
		CSVFileDataParseException(): Exception(){}
		CSVFileDataParseException(string msg) : Exception(msg){}
	};



	CSVFileData() {}
	CSVFileData(string sep_chars) {
		_sep_chars = sep_chars;
	}

	const vector<EntryType>& getEntries() const {
		return _data;
	}

	string getFilename() const {
		return _filename;
	}

	void parseFile(const string filename){
		_filename = filename;
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

			EntryType csv_entry(this, line_no);

			boost::split(values, line, boost::is_any_of(_sep_chars));

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
	string _filename;
	vector<EntryType> _data;
	string _sep_chars = ";,";
};

};

#endif /* CSVFILEDATA_H_ */
