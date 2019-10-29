#include "../include/zmqdatasubscriber.h"
#include <snappy.h>

#include <iostream>

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

void ZmqDataSubscriber::execute()
{
	while (!items.empty())
	{
		std::cout << "ZmqDataSubscriber unprocessed elements: " << items.size() << std::endl;

		auto ad = items.front();
		items.pop_front();
		auto elements = ad.process(0, 0);
		Message msg;

		std::fill(data_vector.begin(), data_vector.end(), 0);

		for (auto& er : *elements)
		{
			msg.add_tic(er.tic);
			msg.add_time_stamps(er.timestamp);

			auto begin = std::begin(er.encoded_spectra);
			auto end = std::end(er.encoded_spectra);

			int index = 0;
			for (auto val : er.encoded_spectra)
			{
				if (val < 0)
				{
					index += (-1 * val);
					continue;
				}
				data_vector[index++] += val;
			}
		}

		*msg.mutable_mz() = { data_vector.begin(), data_vector.end() };

		std::string msg_s;
		msg.SerializeToString(&msg_s);

		std::string compressed_msg_s;
		snappy::Compress(msg_s.data(), msg_s.size(), &compressed_msg_s);

		zmq::message_t to_send(compressed_msg_s.size());
		memcpy((void *)to_send.data(), compressed_msg_s.c_str(), compressed_msg_s.size());

		publisher->send(to_send, subject);
	}
}