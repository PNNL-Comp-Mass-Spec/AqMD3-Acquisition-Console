#ifndef FRAME_ZMQ_SUBSCRIBER_H
#define FRAME_ZMQ_SUBSCRIBER_H

#include "framesubscriber.h"
#include <UIMFWriter/UIMFWriter.h>
#include <deque>
#include <zmq.hpp>

class FrameZmqSubscriber : FrameSubscriber<std::shared_ptr<UimfFrame>> {
private:
	zmq::socket_t socket;
	std::string subject;

public:
	FrameZmqSubscriber(zmq::socket_t data_socket) : FrameSubscriber()
		, socket(std::move(data_socket))
		, subject("data")
	{}

private:
	void execute() override;
};

#endif // !FRAME_WRITER_SUBSCRIBER_H
