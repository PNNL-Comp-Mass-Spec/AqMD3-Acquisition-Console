#ifndef ZMQ_DATA_SUBSCRIBER_H
#define ZMQ_DATA_SUBSCRIBER_H

#include "framesubscriber.h"
#include "acquireddata.h"
#include "server.h"
#include <string>
#include <zmq.hpp>

#include <iostream>

class ZmqDataSubscriber : public FrameSubscriber<AcquiredData> {
private:
	std::shared_ptr<Server::Publisher> publisher;
	std::string subject;
	std::vector<int32_t> data_vector;
	uint64_t processed;

public:
	ZmqDataSubscriber(std::shared_ptr<Server::Publisher> publisher, uint32_t samples)
		: FrameSubscriber()
		, data_vector(samples)
		, publisher(publisher)
		, subject("data")
		, processed(0)
	{}

private:
	void execute() override;
};

#endif // !ZMQ_DATA_SUBSCRIBER_H
