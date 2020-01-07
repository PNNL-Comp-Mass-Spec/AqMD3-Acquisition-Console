#include "../include/processsubject.h"

static std::string finished = "finished";

static uint64_t ts_last = 0;
static uint64_t ts_first = 0;

static int delta = 100;

void ProcessSubject::on_notify() 
{
	while (!items.empty())
	{
		//std::cout << " ProcessSubject unprocessed elements: " << items.size() << std::endl;
		auto ad = items.front();
		items.pop_front();

		ts_first = (ad.stamps.begin())->timestamp;
		//std::cout << "ts diff: " << ts_first - ts_last << std::endl;
		ts_last = (ad.stamps.end() - 1)->timestamp;

		uint64_t avg_ts = 0;
		for (int i = 1; i < ad.stamps.size(); i++)
		{
			avg_ts += (ad.stamps[i].timestamp - ad.stamps[i - 1].timestamp);
			if ((ad.stamps[i].index - ad.stamps[i - 1].index) > 1)
			{
				std::cout << "ad.stamps[i].index: " << ad.stamps[i].index << " --  ad.stamps[i - 1].index: " << ad.stamps[i - 1].index << "\n";
				std::cout << "i: " << i << " --  ad.stamps().size(): " << ad.stamps.size() << std::endl;
				std::cout << "total_triggers_processed: " << total_triggers_processed << std::endl;
			}
		}
		avg_ts /= ad.stamps.size() - 1;

		if (avg_ts > (tof_avg_samples + delta) || avg_ts < (tof_avg_samples - delta))
		{
			std::cout << "timestamps size: " << ad.stamps.size() << std::endl;
			std::cout << "timestamps avg: " << avg_ts << std::endl;
		}

		if (!frames.empty())
		{
			auto frame = frames.front();

			int total_triggers = frame->frame_length * frame->nbr_accumulations;

			if (total_triggers_processed < total_triggers)
			{
				auto results = ad.process(total_triggers_processed % frame->frame_length, frame->offset_bins);
				total_triggers_processed += ad.stamps.size();

				for (int i = 0; i < results->size(); i++)
				{
					total_elements_processed += results->at(i).non_zero_count;
				}

				FramePublisher<segment_ptr>::notify(results, SubscriberType::BOTH);

				auto excess = frame->append_and_return_excess(results);

				if (frame->frame_length == frame->get_encoded_result_count())
				{
					FramePublisher<frame_ptr>::notify(frame, SubscriberType::BOTH);
					frames.pop_front();

					if (excess->size() > 0 && !frames.empty())
					{
						frames.front()->append_and_return_excess(excess);
					}
				}

				if (total_triggers_processed >= total_triggers)
				{
					std::string finished = "finished";
					zmq::message_t finished_msg(finished.size());
					memcpy((void *)finished_msg.data(), finished.c_str(), finished.size());

					publisher->send(finished_msg, subject);
					//auto t2 = std::chrono::high_resolution_clock::now();
				}
			}
		}
		else
		{
			total_triggers_processed += ad.stamps.size();
			try
			{
				//auto t1 = std::chrono::high_resolution_clock::now();
				auto results = ad.process(0, offset_bins);
				//auto t2 = std::chrono::high_resolution_clock::now();
				//auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
				//std::cout << "\tprocess time: " << dur.count() << "\n";

				//auto t1_n = std::chrono::high_resolution_clock::now();
				FramePublisher<segment_ptr>::notify(results, SubscriberType::BOTH);
				//auto t2_n = std::chrono::high_resolution_clock::now();
				//auto dur_n = std::chrono::duration_cast<std::chrono::milliseconds>(t2_n - t1_n);
				//std::cout << "\t\tTIME TO PROCESS NOTIFY: " << dur_n.count() << "\n";
			}
			catch (...)
			{
				std::cout << "processing error" << std::endl;
			}
		}

	}
}