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

public:
	RawPrinterSubscriber(std::string out_file)
		: out_file_stream(out_file)
		, out_file(out_file)
	{}
	~RawPrinterSubscriber()
	{
		out_file_stream.close();
	}

private:
	void execute() override;
};

void RawPrinterSubscriber::execute()
{
	if (!out_file_stream.is_open())
		out_file_stream.open(out_file);

	while (!items.empty())
	{
		auto ad = items.front();
		items.pop_front();
		auto elements = ad.process(0, 0);

		out_file_stream << "-- GATE INFO--" << std::endl;

		for (int i = 0; i < ad.stamps.size(); i++)
		{
			out_file_stream << "\tGATE " << ad.stamps[i].index << std::endl;

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
