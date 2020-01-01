#include "../include/uimfframewritersubscriber.h"

#include<UIMFWriter/UIMFWriter.h>

void UimfFrameWriterSubscriber::on_notify()
{
	if (!items.empty())
	{
		auto frame = items.front();
		items.pop_front();

		UimfWriter writer(frame->file_name);
		try
		{
			auto t1 = std::chrono::high_resolution_clock::now();
			writer.write_scan_data(*frame);
			//writer.update_timing_information(*current_frame, ts_sample_period);
			auto t2 = std::chrono::high_resolution_clock::now();
			auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

			std::cout << "Time to write frame: " << dur.count() << "ms\n";
		}
		catch (...)
		{
			std::cout << "there was an error writing the UIMF frame" << std::endl;
		}
	}
}

void UimfFrameWriterSubscriber::on_completed()
{
}