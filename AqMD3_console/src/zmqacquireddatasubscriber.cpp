#include "../include/zmqacquireddatasubscriber.h"
#include <snappy.h>
#include <iostream>

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

void ZmqAcquiredDataSubscriber::on_notify()
{
	while (!items.empty())
	{
		//std::cout << "VectorZmqWriterSubscriber unprocessed elements: " << items.size() << std::endl;

		auto ad = items.front();
		items.pop_front();

		Message msg;

		std::fill(data_vector.begin(), data_vector.end(), 0);

		for (auto& er : *ad)
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

					if (index >= data_vector.size())
						throw std::string("index oob error -> index: ") + std::to_string(index);

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

		processed++;
	}
}
