#include "../include/framesubject.h"

#include <iostream>

void FrameSubject::execute()
{
	while (!items.empty())
	{
		auto ad = items.front();
		items.pop_front();
		
		if (total_processed < frame->frame_length)
		{
			std::cout << "processing" << std::endl;
			auto elements = ad.process(frame->frame_number, process_index++ * ad.stamps.size());
			total_processed += elements.size();

			frame->append_encoded_results(elements);

			if (total_processed >= frame->frame_length)
			{
				std::cout << "Sending UIMF file w/ contents" << std::endl;
				notify(frame, SubscriberType::ACQUIRE_FRAME);

				std::string finished = "finished";
				zmq::message_t finished_msg(finished.size());
				memcpy((void *)finished_msg.data(), finished.c_str(), finished.size());

				zmq::message_t status_msg(subject.size());
				memcpy(status_msg.data(), subject.data(), subject.size());

				socket.send(status_msg, ZMQ_SNDMORE);
				socket.send(finished_msg, 0);
			}
		}
	}
}