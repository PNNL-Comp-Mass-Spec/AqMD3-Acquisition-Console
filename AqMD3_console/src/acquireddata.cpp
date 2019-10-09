#include "../include/acquireddata.h"

#include <iostream>
#include <map>
#include <numeric>
#include <tuple>
#include <algorithm>

std::vector<EncodedResult> AcquiredData::process(int frame, int processing_scan_start_number) const
{
	std::vector<EncodedResult> results;
	results.reserve(stamps.size());
	
	int32_t *ptr = sample_buffer->get_raw_unprocessed();
	int offset = 0;

	for (int trig_index = 0; trig_index < stamps.size(); trig_index++)
	{
		auto trig = stamps[trig_index];
		auto timestamp = trig.timestamp;

		std::vector<int32_t> encoded_samples;

		uint64_t tic = 0;
		int64_t bpi = 0;
		double bpi_mz = 0.0;
		int32_t index_max_intensity = 0;
		
		int32_t non_zero_count = 0;
		int32_t zero_count = 0;

		size_t gate_count = trig.gate_data.size();
		for (int j = 0 ; j < gate_count; j++)
		{
			auto gate = trig.gate_data[j];
			auto samples = get<2>(gate);
			auto elements = samples / 2;
			auto size_to_process = get<3>(gate);

			int32_t gate_zero_count = 0;
			if (gate_count > 1)
			{
				if (j == 0) {
					gate_zero_count = get<0>(gate) - 1;
				}
				else {
					// get difference from start of current gate to end of previous gate in samples
					auto prev_gate = trig.gate_data[j - 1];
					gate_zero_count = get<0>(gate) - get<1>(prev_gate);
				}
			}
			else {
				gate_zero_count = get<0>(gate) - 1;
			}

			if (gate_zero_count != 0)
			{
				encoded_samples.push_back(-1 * gate_zero_count);
			}

			zero_count += gate_zero_count;

			int processed = 0;
			int32_t *ptr = sample_buffer->get_raw_unprocessed() + offset;
			for (int i = 0; i < elements; i++)
			{
				int32_t first = (ptr[i] << 16) >> 16;
				int32_t second = ptr[i] >> 16;

				// if(first < (zThreshold - zHysteresis))
				if (first < 0)
					first = 0;


				if (first > bpi)
				{
					bpi = first;
					index_max_intensity = non_zero_count + zero_count + (2 * i);
					bpi_mz = index_max_intensity;
				}

				encoded_samples.push_back(first);
				tic += first;

				if (second < 0)
					second = 0;

				if (second > bpi)
				{
					bpi = second;
					index_max_intensity = non_zero_count + zero_count + (2 * i) + 1;
					bpi_mz = index_max_intensity;
				}

				encoded_samples.push_back(second);
				tic += first + second;
				processed += 2;
			}
			//cout << "processed == samples: " << (processed == samples ? "true" : "false") << " processed: " << processed << " samples: " << samples << endl;
			offset += size_to_process;
			non_zero_count += samples;
		}

		results.emplace_back(
			frame,
			processing_scan_start_number + trig_index,
			non_zero_count,
			encoded_samples,
			tic,
			bpi,
			bpi_mz,
			index_max_intensity,
			timestamp);
	}

	sample_buffer_pool->return_in_use(sample_buffer);
	
	return results;
}

//void AcquiredData::process(zmq::socket_t& pub_soc, Message& msg)
//{
//	//std::vector<uint64_t> stamps_val;
//	//std::transform(stamps.begin(), stamps.end(), std::back_inserter(stamps_val), [](SomeShit ss) {return ss.timestamp; });
//
//	//*msg.mutable_time_stamps() = {stamps_val.begin(), stamps_val.end()};
//	//
//	//for ( int32_t* ptr = sample_buffer->get_raw_unprocessed();
//	//	  ptr < sample_buffer->get_raw_unprocessed() + sample_buffer->get_acquired();
//	//	  ptr++
//	//	)
//	//{
//	//	msg.add_mz(*ptr);
//	//}
//	//memcpy(msg.mutable_mz()->mutable_data(), sample_buffer->get_raw_unprocessed(), sizeof(int32_t) * (sample_buffer->get_acquired()));
//
//	//cout << "sending :: " << sample_buffer->get_acquired() << endl;
//
//	// print trigger data
//	//std::map<int, int> frequency;
//
//	//for (auto& elem : stamps)
//	//{
//	//	frequency[elem.gate_cage.size()]++;
//	//}
//
//	//for (auto& kvp : frequency)
//	//{
//	//	std::cout << kvp.first << " :: " << kvp.second << endl;
//	//}
//
//	//std::string msg_s;
//	//msg.SerializeToString(&msg_s);
//
//	//zmq::message_t to_send(msg_s.size());
//	//memcpy((void *)to_send.data(), msg_s.c_str(), msg_s.size());
//
//	//zmq::message_t data2(4);
//	//std::string d_msg2 = "data";
//	//memcpy(data2.data(), d_msg2.c_str(), 4);
//	//pub_soc.send(data2, ZMQ_SNDMORE);
//
//	//pub_soc.send(to_send, 0);
//
//	sample_buffer_pool->return_in_use(sample_buffer);
//}