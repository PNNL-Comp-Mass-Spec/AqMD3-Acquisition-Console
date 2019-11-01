#include "../include/processsubject.h"

void ProcessSubject::execute() 
{
	while (!items.empty())
	{
		//std::cout << " ProcessSubject unprocessed elements: " << items.size() << std::endl;

		auto start_0 = std::chrono::high_resolution_clock::now();
		auto ad = items.front();
		items.pop_front();		
		auto stop_0 = std::chrono::high_resolution_clock::now();
		auto ms_0 = std::chrono::duration_cast<std::chrono::milliseconds>(stop_0 - start_0);
		total_duration += ms_0;

		if (frame)
		{
			int total_triggers = frame->frame_length * frame->nbr_accumulations;
			//int frame_num = (total_triggers_processed / frame->frame_length) + 1;

			if (total_triggers_processed < total_triggers)
			{			
				auto start_1 = std::chrono::high_resolution_clock::now();
				auto results = ad.process(total_triggers_processed % frame->frame_length);
				total_triggers_processed += ad.stamps.size();
				auto stop_1 = std::chrono::high_resolution_clock::now();
				auto ms_1 = std::chrono::duration_cast<std::chrono::milliseconds>(stop_1 - start_1);
				//std::cout << "\t\t\tprocess time: " << ms_1.count()  << "ms" << std::endl;
				
				total_duration += ms_1;
				for (int i = 0; i < results->size(); i++)
				{
					total_elements_processed += results->at(i).non_zero_count;
				}
				
				notify(results, SubscriberType::BOTH);
				process_index = 0;

				if (total_triggers_processed >= total_triggers)
				{
					std::string finished = "finished";
					zmq::message_t finished_msg(finished.size());
					memcpy((void *)finished_msg.data(), finished.c_str(), finished.size());
					
					publisher->send(finished_msg, subject);
				}
			}
		}
		else
		{
			total_triggers_processed += ad.stamps.size();
			auto results = ad.process(0);
			notify(results, SubscriberType::BOTH);
		}
	}
}