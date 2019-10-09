#ifndef DATA_PUBLISHER_H
#define DATA_PUBLISHER_H

#include "acquireddata.h"
#include "acquireddatasink.h"
#include <string>
#include <zmq.hpp>

class DataPublisher : public AcquiredDataSink {
private:
	zmq::socket_t socket;
	uint32_t samples;

public:
	DataPublisher(zmq::socket_t socket, uint32_t samples)
		: socket(std::move(socket))
		, samples(samples)
	{}

	void process(std::vector<EncodedResult> elements) override;
};

#endif // !DATA_PUBLISHER_H
