#ifndef ZMQ_ACQUIRED_DATA_SUBSCRIBER_H
#define ZMQ_ACQUIRED_DATA_SUBSCRIBER_H

#include "framesubscriber.h"
#include "acquireddata.h"
#include "server.h"
#include <string>
#include <zmq.hpp>

#include <iostream>

class ZmqAcquiredDataSubscriber : public FrameSubscriber<std::shared_ptr<std::vector<EncodedResult>>> {
private:
	std::shared_ptr<Server::Publisher> publisher;
	std::string subject;
	std::vector<int32_t> data_vector;
	uint64_t processed;

public:
	ZmqAcquiredDataSubscriber(std::shared_ptr<Server::Publisher> publisher, uint32_t sample_count)
		: FrameSubscriber()
		, data_vector(sample_count)
		, publisher(publisher)
		, subject("data")
		, processed(0)
	{}

private:
	void on_notify() override;
	void on_completed() override
	{
	}
};

#endif // ZMQ_ACQUIRED_DATA_SUBSCRIBER_H
