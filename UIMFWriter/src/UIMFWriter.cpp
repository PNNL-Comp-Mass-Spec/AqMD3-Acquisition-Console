#include "UIMFWriter/UIMFWriter.h"

#include <SQLiteCpp/SQLiteCpp.h>
#include <sqlite3.h>

#include <iostream>
#include <fstream>

const std::string UimfWriter::frames_table_name = "Frame_Params";

void UimfWriter::write_scan_data(const UimfFrame& frame)
{
	std::string insert_scan_statement = "INSERT INTO Frame_Scans (FrameNum, ScanNum, NonZeroCount, BPI, BPI_MZ, TIC, Intensities) VALUES(%d, %d, %d, %d, %lf, %d, ?)";
	int insert_scan_statement_size_bytes = 110;

	const std::lock_guard<std::mutex> lock(sync);

	try
	{
		int const extra = 128;
		SQLite::Transaction transaction(db);
		char *statement = new char[insert_scan_statement_size_bytes + extra];

		for (int i = 0; i < frame.get_data().size(); i++)
		{
			for (auto& er : *(frame.get_data()[i]))
			{
				if (er.scan < frame.start_trigger)
					continue;

				if (er.encoded_spectra.size() > 1)
				{
					int count = sprintf(statement,
						insert_scan_statement.c_str(),
						frame.frame_number,
						er.scan - frame.start_trigger,
						er.non_zero_count,
						er.bpi,
						er.bpi_mz,
						er.tic);

					auto compressed = er.get_compressed_spectra();

					SQLite::Statement statement(db, statement);
					statement.bind(1, compressed.data, compressed.size);
					statement.exec();
				}
			}
		}

		transaction.commit();
	}
	catch (std::exception& e)
	{
		std::cout << "exception: " << e.what() << std::endl;
	}

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
		statement.bind(2, std::to_string(frame.frame_number));
		statement.bind(3, std::to_string(FrameParamKeyType::DurationSeconds));

		statement.exec();
	}
	catch (...)
	{
	}

}
