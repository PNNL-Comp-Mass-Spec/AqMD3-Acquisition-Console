#include "../include/processsubject.h"

static uint64_t ts_last = 0;
static uint64_t ts_first = 0;

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

		//ts_first = (ad.stamps.begin())->timestamp;
		//std::cout << "ts diff: " << ts_first - ts_last << std::endl;
		//ts_last = (ad.stamps.end() - 1)->timestamp;

		//uint64_t avg_ts = 0;
		//for (int i = 1; i < ad.stamps.size(); i++)
		//{
		//	avg_ts += (ad.stamps[i].timestamp - ad.stamps[i - 1].timestamp);
		//}
		//avg_ts /= ad.stamps.size() - 1;

		//std::cout << "timestamps avg: " << avg_ts << std::endl;
 
		if (frame)
		{
			int total_triggers = frame->frame_length * frame->nbr_accumulations;

			if (total_triggers_processed < total_triggers)
			{			
				auto start_1 = std::chrono::high_resolution_clock::now();
				auto results = ad.process(total_triggers_processed % frame->frame_length, frame->offset_bins);
				total_triggers_processed += ad.stamps.size();
				auto stop_1 = std::chrono::high_resolution_clock::now();
				auto ms_1 = std::chrono::duration_cast<std::chrono::milliseconds>(stop_1 - start_1);
				
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
			try
			{
				auto results = ad.process(0, offset_bins);
				notify(results, SubscriberType::BOTH);
			}
			catch (...)
			{
				std::cout << "processing error" << std::endl;
			}
		}
	}
}