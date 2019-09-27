#ifndef STREAMING_CONTEXT_H
#define STREAMING_CONTEXT_H

//#include "digitizer.h"
#include "acquireddata.h"
#include "AqMD3.h"
#include <memory>
#include <chrono>
#include <string>

class StreamingContext {
protected:
	ViSession session;
	AcquisitionBufferPool *samples_buffer_pool;

	std::string markers_channel;
	std::string samples_channel;

	uint64_t samples_per_trigger;
	uint32_t triggers_per_read;
public:
	StreamingContext(ViSession session,
		std::string samples_channel,
		int64_t samples_buffer_size,
		int32_t samples_buffer_count,
		std::string markers_channel,
		uint64_t samples_per_trigger,
		int32_t triggers_per_read)
		: session(session)
		, samples_buffer_pool(new AcquisitionBufferPool(samples_buffer_size, samples_buffer_count))
		, samples_channel(samples_channel)
		, markers_channel(markers_channel)
		, samples_per_trigger(samples_per_trigger)
		, triggers_per_read(triggers_per_read)
	{}

	virtual AcquiredData acquire(std::chrono::milliseconds timeoutMs) = 0;

	void start();
	void stop();
};

#endif // !STREAMING_CONTEXT_H
