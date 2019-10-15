#ifndef FRAME_ZMQ_SUBSCRIBER_H
#define FRAME_ZMQ_SUBSCRIBER_H

#include "framesubscriber.h"
#include <UIMFWriter/UIMFWriter.h>
#include <deque>
#include <zmq.hpp>

class FrameZmqSubscriber : FrameSubscriber {
private:
	zmq::socket_t socket;

public:
	FrameZmqSubscriber(zmq::socket_t socket)
		: socket(std::move(socket))
	{}

private:
	int execute() override;
};

#endif // !FRAME_WRITER_SUBSCRIBER_H
