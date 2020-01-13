#include "..\include\uimfframesubscriber.h"
#include <UIMFWriter/UIMFWriter.h>

void UimfFrameSubscriber::on_notify()
{
	//while (!items.empty())
	//{
	//	//std::cout << " FrameSubject unprocessed elements: " << items.size() << std::endl;

	//	auto results = items.front();
	//	items.pop_front();

	//	if (!frames.empty())
	//	{
	//		auto current_frame = frames.front();
	//		auto excess = current_frame->append_and_return_excess(results);

	//		//std::cout << "current_frame->frame_length" << current_frame->frame_length << std::endl;
	//		//std::cout << "current_frame->get_encoded_result_count()" << current_frame->get_encoded_result_count() << std::endl;
	//		
	//		if (current_frame->frame_length == current_frame->get_encoded_result_count())
	//		{
	//			std::cout << "current_frame->frame_number " << current_frame->frame_number << std::endl;

	//			UimfWriter writer(current_frame->file_name);
	//			std::cout << "start writing frame" << std::endl;
	//			try
	//			{
	//				writer.write_scan_data(*current_frame);
	//				//writer.update_timing_information(*current_frame, ts_sample_period);
	//			}
	//			catch (...)
	//			{
	//				std::cout << "there was an error writing the UIMF frame" << std::endl;
	//			}
	//			std::cout << "end writing frame" << std::endl;
	//			frames.pop_front();

	//			if (excess->size() > 0 && !frames.empty())
	//			{
	//				frames.front()->append_and_return_excess(excess);
	//			}
	//		}
	//	}
	//}

}
