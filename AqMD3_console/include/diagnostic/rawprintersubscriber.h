#ifndef RAW_PRINTER_SUBSCRIBER_H
#define RAW_PRINTER_SUBSCRIBER_H

#include "../framesubscriber.h"
#include <libaqmd3/acquireddata.h>

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

		out_file_stream << "-- RECORD INFO --" << std::endl;

		for (const auto& trig : ad.stamps)
		{
			out_file_stream << "\tTIME STAMP " << trig.index << std::endl;
			out_file_stream << "\t--TIMESTAMP: " << trig.timestamp << std::endl;
			out_file_stream << "\t--SUBSAMPLE POS: " << trig.subsample_pos << std::endl;
			out_file_stream << std::endl;

			for (const auto& gate : trig.gate_data)
			{
				auto start_sample = int64_t(gate.get_start_sample_index());
				auto stop_sample = int64_t(gate.get_stop_sample_index());

				auto diff = stop_sample - start_sample;

				out_file_stream << "\t\tSTART SAMPLE: " << gate.get_start_sample_index() << std::endl;
				out_file_stream << "\t\tSTOP SAMPLE INDEX: " << gate.get_stop_sample_index() << std::endl;
				out_file_stream << "\t\tTOTAL SAMPLES: " << diff << std::endl;
				out_file_stream << std::endl;
			}
		}
	}
}


#endif // !RAW_PRINTER_SUBSCRIBER_H
