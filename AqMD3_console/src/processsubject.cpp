#include "../include/processsubject.h"
#include "../include/definitions.h"
#include "../include/definitions.h"
#include "../include/util//timehelpers.h"
#include <numeric>
#include <format>
#include <spdlog/spdlog.h>

static std::string finished = "finished";
static int delta = 100; // TODO: move to config

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#if TIMING_INFORMATION
std::chrono::steady_clock::time_point start;
static bool first = true;
#endif

void ProcessSubject::on_notify() 
{
	try
	{
		while (!items.empty())
		{
			auto ad = items.front();
			items.pop_front();

			uint64_t avg_ts = 0;
			for (int i = 1; i < ad.stamps.size(); i++)
			{
				avg_ts += (ad.stamps[i].timestamp - ad.stamps[i - 1].timestamp);
				if ((ad.stamps[i].index - ad.stamps[i - 1].index) > 1)
				{
					spdlog::debug("ad.stamps[i].index: " + std::to_string(ad.stamps[i].index) + " --  ad.stamps[i - 1].index: " +  std::to_string(ad.stamps[i - 1].index));
					spdlog::debug("i: " + std::to_string(i) + " --  ad.stamps().size(): " + std::to_string(ad.stamps.size()));
					spdlog::debug("total_triggers_processed: " + std::to_string(total_triggers_processed));
				}
			}
			avg_ts /= ad.stamps.size() - 1;

			if (avg_ts > (tof_avg_samples + delta) || avg_ts < (tof_avg_samples - delta))
			{
				spdlog::warn("Successive recorded trigger event timestamps differ by more than " + std::to_string(delta));
				spdlog::warn("Total timestamps:\t" +  std::to_string(ad.stamps.size()));
				spdlog::warn("timestamps avg:\t" + std::to_string(avg_ts));
			}

			if (!frames.empty())
			{
	#if TIMING_INFORMATION
				if (first)
				{
					start = std::chrono::high_resolution_clock::now();
					std::cout << "START PROCESS: " << timestamp_now() << std::endl;
					first = false;
				}
	#endif

				auto frame = frames.front();

				int total_triggers = frame->frame_length * frame->nbr_accumulations;

				if (total_triggers_processed < total_triggers)
				{
					// Use calculated offset_bins and not frame->offset_bins as the value in the UIMF frame request
					// message might represent the 'TimeOffset' in ns (which is currently hardcoded to be 10000 in Falkor)
					// and not the number of post-trigger samples to reject: 0.00001s -> 20000 samples @ 2GS/s.
					auto results = ad.process(total_triggers_processed % frame->frame_length, offset_bins);
				
					total_triggers_processed += ad.stamps.size();

					for (int i = 0; i < results->size(); i++)
					{
						total_elements_processed += results->at(i).non_zero_count;
					}

					Publisher<segment_ptr>::notify(results, SubscriberType::BOTH);

					auto excess = frame->append_and_return_excess(results);

					auto notify_trigger_count = (int32_t(frame->frame_length) - total_triggers_processed) < (int32_t(frame->frame_length) % notify_on_scans_count)
						? frame->frame_length % notify_on_scans_count
						: notify_on_scans_count;

					if(notify_trigger_count <= frame->get_encoded_results_count())
					{
						frames.push_back(frames.front()->clone());
					
						spdlog::debug(std::format("{} scans processed, notifying-- {}", notify_on_scans_count, timestamp_now()));
						Publisher<frame_ptr>::notify(frame, SubscriberType::BOTH);
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

	#if TIMING_INFORMATION
						auto stop = std::chrono::high_resolution_clock::now();
						auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
						std::cout << "END PROCESS: " << timestamp_now() << " -- DURATION (ms): " << dur.count() << "  -- TOTAL SAMPLES: " << total_elements_processed << std::endl;
						first = true;
	#endif
					}
				}
			}
			else
			{
				total_triggers_processed += ad.stamps.size();

				auto results = ad.process(0, offset_bins);
				
				Publisher<segment_ptr>::notify(results, SubscriberType::BOTH);
			}
		}
	}
	catch (const std::exception& ex)
	{
		spdlog::error("Processing error: " + std::string(ex.what()));
		spdlog::error("Processing subject unprocessed elements: " + std::to_string(items.size()));
	}
	catch (...)
	{
		spdlog::error("Unknown processing error");
		spdlog::error("Processing subject unprocessed elements: " + std::to_string(items.size()));
	}
}