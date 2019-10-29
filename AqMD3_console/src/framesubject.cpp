#include "../include/framesubject.h"

#include <iostream>

void FrameSubject::execute()
{
	while (!items.empty())
	{
		std::cout << " FrameSubject unprocessed elements: " << items.size() << std::endl;

		auto ad = items.front();
		items.pop_front();
		
		if (total_processed < frame->frame_length)
		{
			total_processed += ad.stamps.size();
			frame->append_encoded_results(ad.process(frame->frame_number, process_index++ * ad.stamps.size()));

			if (total_processed >= frame->frame_length)
			{
				notify(frame, SubscriberType::ACQUIRE_FRAME);
			
				std::string finished = "finished";
				zmq::message_t finished_msg(finished.size());
				memcpy((void *)finished_msg.data(), finished.c_str(), finished.size());

				publisher->send(finished_msg, subject);

				notify_completed();
			}
		}
	}
}