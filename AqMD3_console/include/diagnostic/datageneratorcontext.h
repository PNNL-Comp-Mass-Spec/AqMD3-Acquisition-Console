#ifndef DATA_GENERATOR_CONTEXT_H
#define DATA_GENERATOR_CONTEXT_H

#include "libaqmd3/streamingcontext.h"
#include <libaqmd3/AcquiredData.h>
#include <vector>
#include <tuple>
#include <algorithm>
#include <thread>
#include <chrono>

class DataGeneratorContext : public StreamingContext {
private:
	uint64_t tof_width;
	uint64_t acq_count;
	uint64_t triggers_per_read;

public:
	DataGeneratorContext(const Digitizer& digitizer, std::string channel, uint64_t triggers_per_read, std::shared_ptr<AcquisitionBufferPool> buffer_pool)
		: StreamingContext(digitizer, channel, buffer_pool)
		, tof_width(200000)
		, acq_count(0)
		, triggers_per_read(triggers_per_read)
	{}

	void start()
	{
	}

	void stop()
	{
	}

	AcquiredData acquire(std::chrono::milliseconds timeoutMs) override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::vector<AcquiredData::TriggerData> trigger_data;
		
		for (int i = 0; i < triggers_per_read; i++)
		{
			trigger_data.emplace_back(acq_count++ * tof_width, acq_count, 0.0);
			trigger_data.back().gate_data.emplace_back(5000, 0, 5000, 4, 4);
		}

		auto buffer = buffer_pool->get_buffer();
		for (int i = 0; i < 1000; i++)
		{
			auto samples = 0xFFFF0000;
			buffer->get_raw_data()[i] = samples;
		}
		buffer->advance_acquired(1000);

		return AcquiredData(trigger_data, buffer, 400);
	};
};

#endif // !DATA_GENERATOR_CONTEXT_H
