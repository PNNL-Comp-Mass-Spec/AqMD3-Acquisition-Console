#ifndef ZMQ_ACQUIRED_DATA_SUBSCRIBER_H
#define ZMQ_ACQUIRED_DATA_SUBSCRIBER_H

#include "subscriber.h"
#include <libaqmd3/acquireddata.h>
#include "server.h"
#include <string>
#include <zmq.hpp>

#include <iostream>

class ZmqAcquiredDataSubscriber : public Subscriber<std::shared_ptr<std::vector<EncodedResult>>> {
private:
	std::shared_ptr<Server::Publisher> publisher;
	std::string subject;
	std::vector<int32_t> data_vector;
	uint64_t processed;

public:
	ZmqAcquiredDataSubscriber(std::shared_ptr<Server::Publisher> publisher, uint32_t sample_count)
		: Subscriber(true)
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
