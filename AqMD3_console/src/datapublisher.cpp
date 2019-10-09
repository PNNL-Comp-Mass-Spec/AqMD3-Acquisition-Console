#include "..\include\datapublisher.h"

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

#include <numeric>

void DataPublisher::process(std::vector<EncodedResult> elements)
{
	std::cout << "Processing " << elements.size() << " elements" << std::endl;
	Message msg;

	std::vector<int32_t> mz(samples);

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

			mz[index++] = val;
		}
	}

	*msg.mutable_mz() = { mz.begin(), mz.end() };

	std::string msg_s;
	msg.SerializeToString(&msg_s);

	zmq::message_t to_send(msg_s.size());
	memcpy((void *)to_send.data(), msg_s.c_str(), msg_s.size());

	zmq::message_t data2(4);
	std::string d_msg2 = "data";
	memcpy(data2.data(), d_msg2.c_str(), 4);
	socket.send(data2, ZMQ_SNDMORE);

	socket.send(to_send, 0);
}
