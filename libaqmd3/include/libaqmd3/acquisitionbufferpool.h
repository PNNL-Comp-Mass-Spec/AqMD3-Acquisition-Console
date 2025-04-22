#ifndef ACQUISITION_BUFFER_POOL_H
#define ACQUISITION_BUFFER_POOL_H

#include "acquisitionbuffer.h"

class AcquisitionBufferPool
{
private:
	std::vector<std::shared_ptr<AcquisitionBuffer>> samples_buffer_pool;

	uint64_t buffer_takes;
	uint64_t buffer_max_count;
	uint64_t buffer_initial_count;

	uint64_t max_triggers_per_read;
	uint64_t samples_per_trigger;

public:
	AcquisitionBufferPool(uint64_t max_triggers_per_read, uint64_t samples_per_trigger, uint64_t buffer_initial_count, uint64_t buffer_max_count)
		: buffer_takes(0)
		, buffer_max_count(buffer_max_count)
		, buffer_initial_count(buffer_initial_count)
		, max_triggers_per_read(max_triggers_per_read)
		, samples_per_trigger(samples_per_trigger)
	{
		for (int i = 0; i < buffer_initial_count; i++)
		{
			samples_buffer_pool.push_back(std::make_shared<AcquisitionBuffer>(max_triggers_per_read, samples_per_trigger));
		}
	}

	size_t get_available_buffers();
	std::shared_ptr<AcquisitionBuffer> get_buffer();
	uint64_t get_max_triggers_per_read() { return max_triggers_per_read; }

};

#endif //ACQUISITION_BUFFER_POOL_H