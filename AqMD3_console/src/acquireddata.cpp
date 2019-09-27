#include "../include/acquireddata.h"

#include <iostream>
#include <map>
#include <numeric>
#include <tuple>
#include <algorithm>

std::tuple<vector<uint64_t> const, vector<int32_t> const, vector<uint32_t> const> AcquiredData::process() const
{
	int segments = stamps.size();

	vector<uint64_t> timestamps(segments);

	vector<uint32_t> summed_vectors(segments);
	vector<int32_t> summed_samples(samples);

	int32_t *ptr = sample_buffer->get_raw_unprocessed();

	int offset = 0;

	std::cout << "TOTAL SEGMENTS TO PROCESS: " << stamps.size() << "\n";

	for (int k = 0; k < stamps.size(); k++)
	{
		auto trig = stamps[k];
		timestamps[k] = trig.timestamp;

		for (auto& gate : trig.gate_data)
		{
			auto samples_offset_index = get<0>(gate);
			auto size = get<2>(gate) / 2;
			auto size_to_process = get<3>(gate);

			int32_t *ptr = sample_buffer->get_raw_unprocessed() + offset;
			for (int i = 0; i < size; i++)
			{			
				int32_t first = (ptr[i] << 16) >> 16;
				int32_t second = ptr[i] >> 16;

				summed_samples[samples_offset_index + (2 * i)] += first;
				summed_samples[samples_offset_index + (2 * i) + 1] += second;

				summed_vectors[k] += first + second;
			}
			offset += size_to_process;
		}
		offset = 0;
	}

	sample_buffer_pool->return_in_use(sample_buffer);

	return std::make_tuple(timestamps, summed_samples, summed_vectors);
}

//void AcquiredData::process(vector<uint64_t>& timestamps, vector<int32_t>& mz, vector<uint32_t>& tic)
//{
//	int32_t *ptr = sample_buffer->get_raw_unprocessed();
//
//	int offset = 0;
//
//	std::cout << "TOTAL STAMPS TO PROCESS: " << stamps.size() << "\n";
//
//	for (auto& trig : stamps)
//	{
//		timestamps.push_back(trig.timestamp);
//
//		for (auto& gate : trig.gate_data)
//		{
//			auto size = get<2>(gate) / 2;
//			auto size_to_process = get<3>(gate);
//
////			//vector<int32_t> t;
//			//vector<int32_t> vec = accumulate(
//			//	sample_buffer->get_raw_unprocessed() + offset,
//			//	sample_buffer->get_raw_unprocessed() + offset + size,
//			//	mz,
//			//	[](vector<int32_t> vec_l, int32_t val)
//			//	{
//			//		int32_t first = (val << 16) >> 16;
//			//		int32_t second = val >> 16;
//
//			//		vec_l.push_back(first);
//			//		vec_l.push_back(second);
//
//			//		return vec_l;
//			//	});
//
//			//offset += size_to_process;
//			//trig.segments.emplace_back(DataSegment(get<0>(gate), vec));
//
//
//
//			// mz[gate_start + (n * 2) + 0] mz[gate_start + (n * 2) + 1] for n = 0..size
//		}
//	}
//
//	sample_buffer_pool->return_in_use(sample_buffer);
//	//return stamps;
//}
//
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