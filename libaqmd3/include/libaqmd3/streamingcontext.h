#ifndef STREAMING_CONTEXT_H
#define STREAMING_CONTEXT_H

#include "acquireddata.h"
#include "acquisitionbufferpool.h"
#include "AqMD3.h"
#include <memory>
#include <chrono>
#include <string>
#include <atomic>

class Digitizer;

class StreamingContext {
protected:
	std::string const markers_channel;
	std::string const samples_channel;
	const Digitizer& digitizer;
	std::shared_ptr<AcquisitionBufferPool> buffer_pool;

public:
	StreamingContext(const Digitizer& digitizer, std::string channel, std::shared_ptr<AcquisitionBufferPool> buffer_pool)
		: samples_channel(channel == "Channel1" ? "StreamCh1" : "StreamCh2")
		, markers_channel(channel == "Channel1" ? "MarkersCh1" : "MarkersCh2")
		, digitizer(digitizer)
		, buffer_pool(buffer_pool)
	{}

	virtual AcquiredData acquire(uint64_t triggers_to_read, std::chrono::milliseconds timeoutMs) = 0;

	virtual void start();
	virtual void stop();

	bool get_is_acquiring();
};

#endif // !STREAMING_CONTEXT_H
