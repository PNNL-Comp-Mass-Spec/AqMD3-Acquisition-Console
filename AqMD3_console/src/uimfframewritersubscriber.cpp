#include "../include/uimfframewritersubscriber.h"
#include "../include/definitions.h"
#include<UIMFWriter/UIMFWriter.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <format>

void UimfFrameWriterSubscriber::on_notify()
{
	if (!items.empty())
	{
		auto frame = items.front();
		items.pop_front();
		try
		{
			spdlog::info(std::format("UIMF Write frame: {}", frame->frame_number));
			spdlog::info(std::format("UIMF Write file: {}", frame->file_name));
			spdlog::info(std::format("UIMF Write scan count: {}", frame->data.size()));

			auto t1_open = std::chrono::high_resolution_clock::now();
			UimfWriter writer(frame->file_name);
			auto t2_open = std::chrono::high_resolution_clock::now();
			auto dur_open = std::chrono::duration_cast<std::chrono::milliseconds>(t2_open - t1_open);
			spdlog::info(std::format("Time to write (ms): {}", dur_open.count()));

#if TIMING_INFORMATION
			auto t1 = std::chrono::high_resolution_clock::now();
			std::cout << "START FRAME WRITE: " << timestamp_now() << std::endl;
#endif
			int b = 0;
			b = writer.write_scan_data(*frame);

			// Optionally write timestamp information
			if (write_timestamps)
			{
				std::fstream file;
				const std::string ts_file = "timestamps.csv";

				const std::string uimf_file = frame->file_name;
				size_t last_index = uimf_file.find_last_of("\\");
				std::string containing_folder = uimf_file.substr(0, last_index);
				std::string file_name = containing_folder + "\\" + ts_file;

				file.open(file_name, std::ios::app | std::ios::out);

				// Look for first scan in frame
				auto has_first_scan = std::any_of(frame->get_data().begin(), frame->get_data().end(), [](std::shared_ptr<std::vector<EncodedResult>> er) {return er->front().scan == 0; });

				// If this is the first frame, and data contains fist scan, add headers
				if (frame->frame_number == 1 && has_first_scan)
				{
					std::string header = "frame,scan,timestamp\n";
					file.write(header.c_str(), header.length());
				}

				// Write frame number, scan number, and timestamp to file
				size_t buf_len = 256;
				auto buf = new char[buf_len];
				for (auto segments : frame->get_data())
				{
					for (auto& scan : *segments)
					{
						auto size = snprintf(buf, buf_len, "%u,%d,%llu\n", frame->frame_number, scan.scan, scan.timestamp);
						if (size <= 0 || size > buf_len)
						{
							spdlog::error("returned size of snprintf() = " + std::to_string(size));
							continue;
						}
						file.write(buf, size);
					}
				}
			}

#if TIMING_INFORMATION
			auto t2 = std::chrono::high_resolution_clock::now();
			auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
			std::cout << "END FRAME WRITE: " << timestamp_now()
			<< " -- DURATION (ms): " << dur.count()
			<< " -- TOTAL BYTES: " << b
			<< std::endl;
#endif
		}
		catch (SQLite::Exception& ex)
		{
			spdlog::error("Error processing UIMF data: " + std::string(ex.what()));
			if (items.size() > 0)
			{
				spdlog::error("UIMF unprocessed elements: " + std::to_string(items.size()));
			}
		}
		catch (...)
		{
			spdlog::error("Unknown error processing UIMF data");
			if (items.size() > 0)
			{
				spdlog::error("UIMF unprocessed elements: " + std::to_string(items.size()));
			}
		}
	}
}

void UimfFrameWriterSubscriber::on_completed()
{
}