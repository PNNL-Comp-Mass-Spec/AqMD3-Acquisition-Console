#include "../include/processsubject.h"
#include "../include/definitions.h"
#include "../include/definitions.h"
#include "../include/util//timehelpers.h"
#include "../include/uimfacquisitionrecord.h"
#include <numeric>
#include <format>
#include <spdlog/spdlog.h>

//static std::string finished = "finished";
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

			//uint64_t avg_ts = 0;
			//for (int i = 1; i < ad.stamps.size(); i++)
			//{
			//	avg_ts += (ad.stamps[i].timestamp - ad.stamps[i - 1].timestamp);
			//	if ((ad.stamps[i].index - ad.stamps[i - 1].index) > 1)
			//	{
			//		spdlog::debug(std::format("ad.stamps[i].index: {} --  ad.stamps[i - 1].index: {}", ad.stamps[i].index, ad.stamps[i - 1].index));
			//		spdlog::debug(std::format("i: {} --  ad.stamps().size(): {}", i, ad.stamps.size()));
			//		spdlog::debug(std::format("total_triggers_processed: {}", total_triggers_processed));
			//	}
			//}
			//avg_ts /= ad.stamps.size() - 1;

			//// Determine if the initially measured period (in samples) of the clock pulse, tof_avg_samples, differs by more than delta
			//// from the last group of measured scan data's averaged timestamps. Warn if it's the case since the record size was determined using the 
			//// initial value.
			//if (avg_ts > (tof_avg_samples + delta) || avg_ts < (tof_avg_samples - delta))
			//{
			//	spdlog::warn(std::format("Successive recorded trigger event timestamps differ by more than {} samples", delta));
			//	spdlog::warn(std::format("Last {} recorded timestamps avg (in samples):\t {}", ad.stamps.size(), avg_ts));
			//	spdlog::warn(std::format("Expected avg (in samples):\t {} +/- {}", tof_avg_samples, delta));
			//}

			//if (!frames.empty())
			//{
			//	if (total_triggers_processed < frames.front()->frame_length)
			//	{
			//		// Use calculated offset_bins and not frame->offset_bins as the value in the UIMF frame request
			//		// message might represent the 'TimeOffset' in ns (which is currently hardcoded to be 10000 in Falkor)
			//		// and not the number of post-trigger samples to reject: 0.00001s -> 20000 samples @ 2GS/s.
			//		auto results = ad.process(total_triggers_processed % frames.front()->frame_length, offset_bins);

			//		for (int i = 0; i < results->size(); i++)
			//		{
			//			total_elements_processed += results->at(i).non_zero_count;
			//		}

			//		Publisher<segment_ptr>::notify(results, SubscriberType::BOTH);
			//		
			//		while (results->size() > 0)
			//		{
			//			auto scans_needed_emit_count = (uint64_t)MIN(notify_on_scans_count - frames.front()->get_encoded_results_count(), frames.front()->frame_length - total_triggers_processed);

			//			if (results->size() < scans_needed_emit_count)
			//			{
			//				frames.front()->append_encoded_results(results);
			//				total_triggers_processed += results->size();
			//				results = std::make_shared<std::vector<EncodedResult>>();
			//			}
			//			else
			//			{
			//				frames.front()->append_encoded_results(std::make_shared<std::vector<EncodedResult>>(std::begin(*results), std::begin(*results) + scans_needed_emit_count));
			//				total_triggers_processed += scans_needed_emit_count;
			//				frames.push_back(frames.front()->clone());
			//				Publisher<frame_ptr>::notify(frames.front(), SubscriberType::BOTH);
			//				frames.pop_front();
			//				if (total_triggers_processed < frames.front()->frame_length)
			//				{
			//					results = std::make_shared<std::vector<EncodedResult>>(std::begin(*results) + scans_needed_emit_count, std::end(*results));
			//				}
			//				else
			//				{
			//					results = std::make_shared<std::vector<EncodedResult>>();
			//				}
			//			}
			//		}

			//		if (total_triggers_processed >= frames.front()->frame_length)
			//		{
			//			std::string finished = "finished";
			//			zmq::message_t finished_msg(finished.size());
			//			memcpy((void *)finished_msg.data(), finished.c_str(), finished.size());

			//			spdlog::info("Notifying processing complete");
			//			auto start = std::chrono::high_resolution_clock::now();
			//			publisher->send(finished_msg, subject, std::chrono::milliseconds::max());
			//			auto stop = std::chrono::high_resolution_clock::now();
			//			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
			//			spdlog::info("Total time to notify: {} ms", duration.count());
			//			spdlog::info("Total scans processed: {}", total_triggers_processed);

			//			spdlog::debug(std::format("unprocessed buffers count: {}", items.size()));
			//			spdlog::debug(std::format("unprocessed partial frames count: {}", frames.size()));
			//			spdlog::debug(std::format("unprocessed segments count: {}", results->size()));

			//			items.clear();
			//			frames.clear();
			//			results->clear();
			//		}
			//	}
			//}
			//else
			//{
			//	total_triggers_processed += ad.stamps.size();

			//	auto results = ad.process(0, offset_bins);
			//	
			//	Publisher<segment_ptr>::notify(results, SubscriberType::BOTH);
			//}

			auto result = ad.to_frame();
			auto notify_type = result->parameters().file_name.empty() ? SubscriberType::ACQUIRE : SubscriberType::BOTH;
			Publisher<frame_ptr>::notify(result, notify_type);
		}
	}
	catch (const std::exception& ex)
	{
		spdlog::error(std::format("Processing error: {}", ex.what()));
		spdlog::error(std::format("Processing subject unprocessed elements: ", items.size()));
	}
	catch (...)
	{
		spdlog::error("Unknown processing error");
		spdlog::error(std::format("Processing subject unprocessed elements: ", items.size()));
	}
}