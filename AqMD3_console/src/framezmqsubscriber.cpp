#include "..\include\framezmqsubscriber.h"
#include <vector>

#define NOMINMAX 
#undef min
#undef max
#include "../include/message.pb.h"

void FrameZmqSubscriber::execute()
{
	while (!items.empty())
	{
		auto frame = items.front();
		items.pop_front();
		
		Message msg;
		msg.mutable_tic()->Reserve(frame->frame_length);
		msg.mutable_time_stamps()->Reserve(frame->frame_length);
		std::vector<int32_t> mz(frame->nbr_samples);

		auto data = frame->get_data();
		for (int i = 0; i < frame->frame_length; i++)
		{
			(*msg.mutable_mz())[i] = data[i].tic;
			(*msg.mutable_time_stamps())[i] = data[i].timestamp;

			int index = 0;
			for (auto val : data[i].encoded_spectra)
			{
				if (val < 0)
				{
					index += (-1 * val);
					continue;
				}

				mz[index++] += val;
			}
		}

		*msg.mutable_mz() = { mz.begin(), mz.end() };

		std::string msg_s;
		msg.SerializeToString(&msg_s);

		zmq::message_t to_send(msg_s.size());
		memcpy((void *)to_send.data(), msg_s.c_str(), msg_s.size());

		zmq::message_t sub_data(4);
		std::string d_msg2 = "data";
		memcpy(sub_data.data(), d_msg2.c_str(), 4);

		socket.send(sub_data, ZMQ_SNDMORE);
		socket.send(to_send, 0);
	}
}
