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
#include <mutex>

class UimfWriter {
private:
	static const std::string frames_table_name;

	std::mutex sync;
	SQLite::Database db;


public:
	UimfWriter(std::string file) 
		: db(file, SQLite::OPEN_READWRITE)
	{}
	~UimfWriter()
	{}

	void write_scan_data(const UimfFrame& frame);
	void update_timing_information(const UimfFrame& frame, double timestamp_sample_period_s);

private:
	enum FrameParamKeyType
	{
		StartTimeMinutes = 1,
		DurationSeconds = 2,
	};

};

#endif // !UIMFWRITER_H
