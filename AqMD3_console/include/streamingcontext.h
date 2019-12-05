#ifndef STREAMING_CONTEXT_H
#define STREAMING_CONTEXT_H

//#include "digitizer.h"
#include "acquireddata.h"
#include "AqMD3.h"
#include <memory>
#include <chrono>
#include <string>
#include <atomic>

class Digitizer;

class StreamingContext {
protected:
	std::vector<std::shared_ptr<AcquisitionBuffer>> samples_buffer_pool;

	std::string markers_channel;
	std::string samples_channel;

	int64_t samples_buffer_size;
	uint64_t samples_per_trigger;
	uint32_t triggers_per_read;

	int buffer_takes;
	int const buffer_max_count = 100;

	std::atomic_bool should_stop;

	Digitizer& digitizer;

public:
	StreamingContext(Digitizer& digitizer,
		std::string channel,
		int64_t samples_buffer_size,
		int32_t samples_buffer_count,
		uint64_t samples_per_trigger,
		int32_t triggers_per_read)
		: samples_channel(channel == "Channel1" ? "StreamCh1" : "StreamCh2")
		, markers_channel(channel == "Channel1" ? "MarkersCh1" : "MarkersCh2")
		, samples_per_trigger(samples_per_trigger)
		, triggers_per_read(triggers_per_read)
		, buffer_takes(0)
		, samples_buffer_pool()
		, samples_buffer_size(samples_buffer_size)
		, should_stop(false)
		, digitizer(digitizer)
	{
		for(int i = 0; i < samples_buffer_count; i++)
			samples_buffer_pool.push_back(std::make_shared<AcquisitionBuffer>(i, samples_buffer_size));
	}

	virtual AcquiredData acquire(std::chrono::milliseconds timeoutMs) = 0;

	void start();
	void stop();

	inline bool is_stopped() { return should_stop; }

protected:
	std::shared_ptr<AcquisitionBuffer> get_buffer();
};

#endif // !STREAMING_CONTEXT_H
