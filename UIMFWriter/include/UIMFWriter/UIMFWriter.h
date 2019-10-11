#ifndef UIMFWRITER_H
#define UIMFWRITER_H

#include "encodedresult.h"
#include "uimfframe.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <tuple>

class UimfWriter {
private:
	SQLite::Database db;

	std::string const insert_scan_statement;
	size_t const insert_scan_statement_size_bytes;

public:
	UimfWriter(std::string file) 
		: db(file, SQLite::OPEN_READWRITE)
		, insert_scan_statement("INSERT INTO Frame_Scans (FrameNum, ScanNum, NonZeroCount, BPI, BPI_MZ, TIC, Intensities) VALUES(%d, %d, %d, %d, %lf, %d, ?)")
		, insert_scan_statement_size_bytes(110)
	{}
	~UimfWriter()
	{}

	void write_frame(UimfFrame& results);
};

#endif // !UIMFWRITER_H
