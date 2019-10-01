#ifndef UIMFWRITER_H
#define UIMFWRITER_H

#include "encodedresult.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>
#include <vector>

class UimfWriter {
private:
	std::string const insert_scan_statement;
	size_t const insert_scan_statement_size_bytes;

	SQLite::Database db;

	std::string encoded_result_to_insert_statement(const EncodedResult& er) const;

public:
	UimfWriter(std::string database) 
		: db(database, SQLite::OPEN_READWRITE)
		, insert_scan_statement("INSERT INTO Frame_Scans (FrameNum, ScanNum, NonZeroCount, BPI, BPI_MZ, TIC, Intensities) VALUES(%d, %d, %d, %d, %lf, %d, %s);")
		, insert_scan_statement_size_bytes(111)
	{}
	~UimfWriter() 
	{
	}
	
	void write_encoded_results(std::vector<EncodedResult> results);
};

#endif // !UIMFWRITER_H
