#include "../include/acquireddata.h"



void AcquiredData::process(zmq::socket_t& pub_soc, Message& msg)
{
	*msg.mutable_time_stamps() = {stamps.begin(), stamps.end()};
	
	for ( int32_t* ptr = sample_buffer->get_raw_unprocessed();
		  ptr < sample_buffer->get_raw_unprocessed() + sample_buffer->get_acquired();
		  ptr++
		)
	{
		msg.add_mz(*ptr);
	}
	//memcpy(msg.mutable_mz()->mutable_data(), sample_buffer->get_raw_unprocessed(), sizeof(int32_t) * (sample_buffer->get_acquired()));

	cout << "sending :: " << sample_buffer->get_acquired() << endl;
	std::string msg_s;
	msg.SerializeToString(&msg_s);

	zmq::message_t to_send(msg_s.size());
	memcpy((void *)to_send.data(), msg_s.c_str(), msg_s.size());

	zmq::message_t data2(4);
	std::string d_msg2 = "data";
	memcpy(data2.data(), d_msg2.c_str(), 4);
	pub_soc.send(data2, ZMQ_SNDMORE);

	pub_soc.send(to_send, 0);

	sample_repo->return_in_use(sample_buffer);
}