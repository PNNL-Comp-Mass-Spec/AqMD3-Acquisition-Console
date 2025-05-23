#include "UIMFWriter/UIMFWriter.h"
#include <SQLiteCpp/SQLiteCpp.h>
#include <iostream>
#include <fstream>

const std::string UimfWriter::frames_table_name = "Frame_Params";

UimfWriter::UimfWriter(std::string file)
	: db(file, SQLite::OPEN_READWRITE)
{}

int UimfWriter::write_scan_data(const UimfFrame& frame)
{
	int bytes = 0;
	std::string insert_scan_statement = "INSERT INTO Frame_Scans (FrameNum, ScanNum, NonZeroCount, BPI, BPI_MZ, TIC, Intensities) VALUES(%d, %d, %d, %d, %lf, %d, ?)";
	int insert_scan_statement_size_bytes = 110;

	const std::lock_guard<std::mutex> lock(sync);

	int const extra = 128;

	SQLite::Statement sync_off(db, "PRAGMA synchronous=0");
	sync_off.exec();
	SQLite::Statement b_trans(db, "BEGIN TRANSACTION");
	b_trans.exec();
		
	char *statement = new char[insert_scan_statement_size_bytes + extra];

	for (auto& er : frame.data())
	{
		if (er.scan < frame.parameters().start_trigger)
			continue;

		if (er.encoded_spectra.size() > 1 || er.scan == 0)
		{
			int count = sprintf(statement,
				insert_scan_statement.c_str(),
				frame.parameters().frame_number,
				er.scan - frame.parameters().start_trigger,
				er.non_zero_count,
				er.bpi,
				er.bpi_mz,
				er.tic);

			auto compressed = er.get_compressed_spectra();

			bytes += compressed.size + count;

			SQLite::Statement sql_statement(db, (const char *)statement);
			sql_statement.bind(1, compressed.data, compressed.size);
			sql_statement.exec();
		}
	}

	SQLite::Statement e_trans(db, "END TRANSACTION");
	e_trans.exec();
	SQLite::Statement sync_on(db, "PRAGMA synchronous=1");
	sync_on.exec();

	return bytes;
}

void UimfWriter::update_timing_information(const UimfFrame& frame, double timestamp_sample_period_s)
{
	std::string frame_update_statement = "UPDATE Frame_Params SET ParamValue = ? WHERE FrameNum = ? AND ParamID = ?";
	SQLite::Statement statement(db, frame_update_statement);

	const std::lock_guard<std::mutex> lock(sync);

	try
	{
		auto frame_duration_seconds = frame.get_frame_duration_seconds(timestamp_sample_period_s);
		statement.bind(1, std::to_string(frame_duration_seconds));
		statement.bind(2, std::to_string(frame.parameters().frame_number));
		statement.bind(3, std::to_string(FrameParamKeyType::DurationSeconds));

		statement.exec();
	}
	catch (...)
	{
	}

}
