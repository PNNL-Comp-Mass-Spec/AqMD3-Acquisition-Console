#ifndef ZMQ_DATA_SUBSCRIBER_H
#define ZMQ_DATA_SUBSCRIBER_H

#include "framesubscriber.h"
#include "acquireddata.h"
#include <string>
#include <zmq.hpp>

class ZmqDataSubscriber : public FrameSubscriber<AcquiredData> {
private:
	zmq::socket_t socket;
	std::string subject;

	std::vector<int32_t> data_vector;

public:
	ZmqDataSubscriber(zmq::socket_t socket, uint32_t samples)
		: FrameSubscriber()
		, data_vector(samples)
		, socket(std::move(socket))
		, subject("data")
	{}

private:
	void execute() override;
};

#endif // !ZMQ_DATA_SUBSCRIBER_H
