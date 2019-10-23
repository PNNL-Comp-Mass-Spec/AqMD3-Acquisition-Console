#include "../include/zmqdatasubscriber.h"
#include <snappy.h>

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

void ZmqDataSubscriber::execute()
{
	while (!items.empty())
	{
		std::cout << "ACQUIRED DATA UNPROCESSED: " << items.size() << std::endl;

		std::cout << "sig'd" << std::endl;
		int samples = 0;
		auto ad = items.front();
		items.pop_front();
		auto elements = ad.process(0, 0);

		std::cout << "Processing " << elements.size() << " elements" << std::endl;
		Message msg;

		std::fill(data_vector.begin(), data_vector.end(), 0);

		for (auto& er : elements)
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

		zmq::message_t sub_data(4);
		memcpy(sub_data.data(), subject.data(), subject.size());

		socket.send(sub_data, ZMQ_SNDMORE);
		socket.send(to_send, 0);
	}
}