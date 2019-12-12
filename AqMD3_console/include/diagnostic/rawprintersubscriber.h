#ifndef RAW_PRINTER_SUBSCRIBER_H
#define RAW_PRINTER_SUBSCRIBER_H

#include "../framesubscriber.h"
#include "../acquireddata.h"

#include <string>
#include <fstream>

class RawPrinterSubscriber : public FrameSubscriber<AcquiredData> 
{
private:
	std::ofstream out_file_stream;
	std::string out_file;
	uint32_t offset_samples;

public:
	RawPrinterSubscriber(std::string out_file, uint32_t offset_samples)
		: out_file_stream(out_file)
		, out_file(out_file)
		, offset_samples(offset_samples)
	{}
	~RawPrinterSubscriber()
	{
		out_file_stream.close();
	}

private:
	void on_notify() override;
	void on_completed() override {};
};

void RawPrinterSubscriber::on_notify()
{
	if (!out_file_stream.is_open())
		out_file_stream.open(out_file);

	while (!items.empty())
	{
		auto ad = items.front();
		items.pop_front();
		auto elements = ad.process(0, offset_samples);

		out_file_stream << "-- RECORD INFO --" << std::endl;

		for (int i = 0; i < ad.stamps.size(); i++)
		{
			out_file_stream << "\tTIME STAMP " << ad.stamps[i].index << std::endl;
			out_file_stream << "\t--INDEX: " << ad.stamps[i].index << std::endl;
			out_file_stream << "\t--TIMESTAMP: " << ad.stamps[i].timestamp << std::endl;
			out_file_stream << "\t--SUBSAMPLE POS: " << ad.stamps[i].subsample_pos << std::endl;
			out_file_stream << std::endl;

			for (int j = 0; j < ad.stamps[i].gate_data.size(); j++)
			{
				out_file_stream << "\t\tSTART BLOCK INDEX: " << ad.stamps[i].gate_data[j].gate_start_block_index << std::endl;
				out_file_stream << "\t\tSTOP BLOCK INDEX: " << ad.stamps[i].gate_data[j].gate_stop_block_index << std::endl;
				out_file_stream << "\t\tBLOCK INDEX DIFFERENCE: " << int64_t(ad.stamps[i].gate_data[j].gate_stop_block_index) - int64_t(ad.stamps[i].gate_data[j].gate_start_block_index) << std::endl;;
				out_file_stream << "\t\tSTART SAMPLE INDEX: " << ad.stamps[i].gate_data[j].get_start_sample_index() << std::endl;
				out_file_stream << "\t\tSTOP SAMPLE INDEX: " << ad.stamps[i].gate_data[j].get_stop_sample_index() << std::endl;
				out_file_stream << "\t\tSTOP INDEX - START INDEX: " << int64_t(ad.stamps[i].gate_data[j].get_stop_sample_index()) - int64_t(ad.stamps[i].gate_data[j].get_start_sample_index()) << std::endl;
				out_file_stream << std::endl;
			}
		}
	}
}


#endif // !RAW_PRINTER_SUBSCRIBER_H
