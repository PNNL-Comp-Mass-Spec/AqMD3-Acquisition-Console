#include "../include/uimfframewritersubscriber.h"
#include "../include/definitions.h"
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
#if TIMING_INFORMATION
			auto t1 = std::chrono::high_resolution_clock::now();
#endif
			writer.write_scan_data(*frame);
			//writer.update_timing_information(*current_frame, ts_sample_period);
#if TIMING_INFORMATION
			auto t2 = std::chrono::high_resolution_clock::now();
			auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
			std::cout << "frame:" << frame->frame_number << "; time to write frame:" << dur.count() << "\n";
#endif
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