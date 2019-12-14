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
			writer.write_scan_data(*frame);
			//writer.update_timing_information(*current_frame, ts_sample_period);
		}
		catch (...)
		{
			std::cout << "there was an error writing the UIMF frame" << std::endl;
		}
	}
}

void UimfFrameWriterSubscriber::on_completed()
{
	// smile for the camera
}